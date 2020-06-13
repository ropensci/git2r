/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2020 The git2r contributors
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License, version 2,
 *  as published by the Free Software Foundation.
 *
 *  git2r is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <R_ext/Visibility.h>
#include <git2.h>

#include "git2r_arg.h"
#include "git2r_deprecated.h"
#include "git2r_error.h"
#include "git2r_repository.h"
#include "git2r_S3.h"
#include "git2r_tree.h"

/**
 * Init slots in S3 class git_tree
 *
 * @param source a tree
 * @param repo S3 class git_repository that contains the tree
 * @param dest S3 class git_tree to initialize
 * @return void
 */
void attribute_hidden
git2r_tree_init(
    const git_tree *source,
    SEXP repo,
    SEXP dest)
{
    SEXP filemode, id, type, name;
    int *filemode_ptr;
    size_t i, n;
    const git_oid *oid;
    char sha[GIT_OID_HEXSZ + 1];
    const git_tree_entry *entry;

    oid = git_tree_id(source);
    git_oid_tostr(sha, sizeof(sha), oid);
    SET_VECTOR_ELT(dest, git2r_S3_item__git_tree__sha, Rf_mkString(sha));

    n = git_tree_entrycount(source);
    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_tree__filemode,
        filemode = Rf_allocVector(INTSXP, n));

    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_tree__id,
        id = Rf_allocVector(STRSXP, n));

    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_tree__type,
        type = Rf_allocVector(STRSXP, n));

    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_tree__name,
        name = Rf_allocVector(STRSXP, n));

    filemode_ptr = INTEGER(filemode);
    for (i = 0; i < n; ++i) {
        entry = git_tree_entry_byindex(source, i);
        git_oid_tostr(sha, sizeof(sha), git_tree_entry_id(entry));
        filemode_ptr[i] = git_tree_entry_filemode(entry);
        SET_STRING_ELT(id,   i, Rf_mkChar(sha));
        SET_STRING_ELT(type, i, Rf_mkChar(git_object_type2string(git_tree_entry_type(entry))));
        SET_STRING_ELT(name, i, Rf_mkChar(git_tree_entry_name(entry)));
    }

    SET_VECTOR_ELT(dest, git2r_S3_item__git_tree__repo, Rf_duplicate(repo));
}

/**
 * Data structure to hold information for the tree traversal.
 */
typedef struct {
    size_t n;
    SEXP list;
    int recursive;
    git_repository *repository;
} git2r_tree_walk_cb_data;

/**
 * Callback for the tree traversal method.
 *
 */
static int
git2r_tree_walk_cb(
    const char *root,
    const git_tree_entry *entry,
    void *payload)
{
    int error = 0;
    git2r_tree_walk_cb_data *p = (git2r_tree_walk_cb_data*)payload;

    if (p->recursive) {
        if (git_tree_entry_type(entry) != GIT2R_OBJECT_BLOB)
            return 0;
    } else if (*root) {
        return 1;
    }

    if (!Rf_isNull(p->list)) {
        char mode[23]; /* enums are int/32-bit, but this is enough for even a 64-bit int */
        git_object *blob = NULL, *obj = NULL;
        char sha[GIT_OID_HEXSZ + 1];

        /* mode */
        error = snprintf(mode, sizeof(mode), "%06o", git_tree_entry_filemode(entry));
        if (error < 0 || (size_t)error >= sizeof(mode)) {
            error = -1;
            goto cleanup;
        }
        SET_STRING_ELT(VECTOR_ELT(p->list, 0), p->n,
                       Rf_mkChar(mode));

        /* type */
        SET_STRING_ELT(VECTOR_ELT(p->list, 1), p->n,
                       Rf_mkChar(git_object_type2string(git_tree_entry_type(entry))));

        /* sha */
        git_oid_tostr(sha, sizeof(sha), git_tree_entry_id(entry));
        SET_STRING_ELT(VECTOR_ELT(p->list, 2), p->n, Rf_mkChar(sha));

        /* path */
        SET_STRING_ELT(VECTOR_ELT(p->list, 3), p->n, Rf_mkChar(root));

        /* name */
        SET_STRING_ELT(VECTOR_ELT(p->list, 4), p->n,
                       Rf_mkChar(git_tree_entry_name(entry)));

        /* length */
        if (git_tree_entry_type(entry) == GIT2R_OBJECT_BLOB) {
            error = git_tree_entry_to_object(&obj, p->repository, entry);
            if (error)
                goto cleanup;
            error = git_object_peel(&blob, obj, GIT2R_OBJECT_BLOB);
            if (error)
                goto cleanup;
            INTEGER(VECTOR_ELT(p->list, 5))[p->n] = git_blob_rawsize((git_blob *)blob);
        } else {
            INTEGER(VECTOR_ELT(p->list, 5))[p->n] = NA_INTEGER;
        }

    cleanup:
        git_object_free(obj);
        git_object_free(blob);
    }

    p->n += 1;

    return error;
}

/**
 * Traverse the entries in a tree and its subtrees.
 *
 * @param tree S3 class git_tree
 * @param recursive recurse into sub-trees.
 * @return A list with entries
 */
SEXP attribute_hidden
git2r_tree_walk(
    SEXP tree,
    SEXP recursive)
{
    int error, nprotect = 0;
    git_oid oid;
    git_tree *tree_obj = NULL;
    git_repository *repository = NULL;
    git2r_tree_walk_cb_data cb_data = {0, R_NilValue};
    SEXP repo = R_NilValue, sha = R_NilValue;
    SEXP result = R_NilValue, names = R_NilValue;

    if (git2r_arg_check_tree(tree))
        git2r_error(__func__, NULL, "'tree'", git2r_err_tree_arg);
    if (git2r_arg_check_logical(recursive))
        git2r_error(__func__, NULL, "'recursive'", git2r_err_logical_arg);

    repo = git2r_get_list_element(tree, "repo");
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    sha = git2r_get_list_element(tree, "sha");
    git_oid_fromstr(&oid, CHAR(STRING_ELT(sha, 0)));
    error = git_tree_lookup(&tree_obj, repository, &oid);
    if (error)
        goto cleanup;

    /* Count number of entries before creating the list */
    cb_data.repository = repository;
    if (LOGICAL(recursive)[0])
        cb_data.recursive = 1;
    error = git_tree_walk(tree_obj, 0, &git2r_tree_walk_cb, &cb_data);
    if (error)
        goto cleanup;

    PROTECT(result = Rf_allocVector(VECSXP, 6));
    nprotect++;
    Rf_setAttrib(result, R_NamesSymbol, names = Rf_allocVector(STRSXP, 6));

    SET_VECTOR_ELT(result, 0,   Rf_allocVector(STRSXP,  cb_data.n));
    SET_STRING_ELT(names,  0, Rf_mkChar("mode"));
    SET_VECTOR_ELT(result, 1,   Rf_allocVector(STRSXP,  cb_data.n));
    SET_STRING_ELT(names,  1, Rf_mkChar("type"));
    SET_VECTOR_ELT(result, 2,   Rf_allocVector(STRSXP,  cb_data.n));
    SET_STRING_ELT(names,  2, Rf_mkChar("sha"));
    SET_VECTOR_ELT(result, 3,   Rf_allocVector(STRSXP,  cb_data.n));
    SET_STRING_ELT(names,  3, Rf_mkChar("path"));
    SET_VECTOR_ELT(result, 4,   Rf_allocVector(STRSXP,  cb_data.n));
    SET_STRING_ELT(names,  4, Rf_mkChar("name"));
    SET_VECTOR_ELT(result, 5,   Rf_allocVector(INTSXP,  cb_data.n));
    SET_STRING_ELT(names,  5, Rf_mkChar("len"));

    cb_data.list = result;
    cb_data.n = 0;
    error = git_tree_walk(tree_obj, 0, &git2r_tree_walk_cb, &cb_data);

cleanup:
    git_repository_free(repository);
    git_tree_free(tree_obj);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}
