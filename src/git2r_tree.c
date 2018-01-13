/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2018 The git2r contributors
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

#include <Rdefines.h>
#include "buffer.h"

#include "git2r_arg.h"
#include "git2r_error.h"
#include "git2r_repository.h"
#include "git2r_tree.h"

/**
 * Init slots in S4 class git_tree
 *
 * @param source a tree
 * @param repo S4 class git_repository that contains the tree
 * @param dest S4 class git_tree to initialize
 * @return void
 */
void git2r_tree_init(const git_tree *source, SEXP repo, SEXP dest)
{
    SEXP filemode, id, type, name;
    int *filemode_ptr;
    size_t i, n;
    const git_oid *oid;
    char sha[GIT_OID_HEXSZ + 1];
    const git_tree_entry *entry;
    SEXP s_sha = Rf_install("sha");
    SEXP s_filemode = Rf_install("filemode");
    SEXP s_id = Rf_install("id");
    SEXP s_type = Rf_install("type");
    SEXP s_name = Rf_install("name");
    SEXP s_repo = Rf_install("repo");

    oid = git_tree_id(source);
    git_oid_tostr(sha, sizeof(sha), oid);
    SET_SLOT(dest, s_sha, mkString(sha));

    n = git_tree_entrycount(source);
    PROTECT(filemode = Rf_allocVector(INTSXP, n));
    SET_SLOT(dest, s_filemode, filemode);
    PROTECT(id = Rf_allocVector(STRSXP, n));
    SET_SLOT(dest, s_id, id);
    PROTECT(type = Rf_allocVector(STRSXP, n));
    SET_SLOT(dest, s_type, type);
    PROTECT(name = Rf_allocVector(STRSXP, n));
    SET_SLOT(dest, s_name, name);

    filemode_ptr = INTEGER(filemode);
    for (i = 0; i < n; ++i) {
        entry = git_tree_entry_byindex(source, i);
        git_oid_tostr(sha, sizeof(sha), git_tree_entry_id(entry));
        filemode_ptr[i] = git_tree_entry_filemode(entry);
        SET_STRING_ELT(id,   i, mkChar(sha));
        SET_STRING_ELT(type, i, mkChar(git_object_type2string(git_tree_entry_type(entry))));
        SET_STRING_ELT(name, i, mkChar(git_tree_entry_name(entry)));
    }

    SET_SLOT(dest, s_repo, repo);
    UNPROTECT(4);
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
static int git2r_tree_walk_cb(
    const char *root,
    const git_tree_entry *entry,
    void *payload)
{
    int err = 0;
    git2r_tree_walk_cb_data *p = (git2r_tree_walk_cb_data*)payload;

    if (!p->recursive && *root)
        return 1;

    if (!Rf_isNull(p->list)) {
        git_buf mode = GIT_BUF_INIT;
        git_object *blob = NULL, *obj = NULL;
        char sha[GIT_OID_HEXSZ + 1];

        /* mode */
        err = git_buf_printf(&mode, "%06o", git_tree_entry_filemode(entry));
        if (err)
            goto cleanup;
        SET_STRING_ELT(VECTOR_ELT(p->list, 0), p->n,
                       mkChar(git_buf_cstr(&mode)));

        /* type */
        SET_STRING_ELT(VECTOR_ELT(p->list, 1), p->n,
                       mkChar(git_object_type2string(git_tree_entry_type(entry))));

        /* sha */
        git_oid_tostr(sha, sizeof(sha), git_tree_entry_id(entry));
        SET_STRING_ELT(VECTOR_ELT(p->list, 2), p->n, mkChar(sha));

        /* path */
        SET_STRING_ELT(VECTOR_ELT(p->list, 3), p->n, mkChar(root));

        /* name */
        SET_STRING_ELT(VECTOR_ELT(p->list, 4), p->n,
                       mkChar(git_tree_entry_name(entry)));

        /* length */
        if (git_tree_entry_type(entry) == GIT_OBJ_BLOB) {
            err = git_tree_entry_to_object(&obj, p->repository, entry);
            if (err)
                goto cleanup;
            err = git_object_peel(&blob, obj, GIT_OBJ_BLOB);
            if (err)
                goto cleanup;
            INTEGER(VECTOR_ELT(p->list, 5))[p->n] = git_blob_rawsize((git_blob *)blob);
        } else {
            INTEGER(VECTOR_ELT(p->list, 5))[p->n] = NA_INTEGER;
        }

    cleanup:
        git_buf_free(&mode);
        git_object_free(obj);
        git_object_free(blob);
    }

    p->n += 1;

    return err;
}

/**
 * Traverse the entries in a tree and its subtrees.
 *
 * @param tree S4 class git_tree
 * @param recursive recurse into sub-trees.
 * @return A list with entries
 */
SEXP git2r_tree_walk(SEXP tree, SEXP recursive)
{
    int err, nprotect = 0;
    git_oid oid;
    git_tree *tree_obj = NULL;
    git_repository *repository = NULL;
    git2r_tree_walk_cb_data cb_data = {0, R_NilValue};
    SEXP repo, sha, result, names;

    if (git2r_arg_check_tree(tree))
        git2r_error(__func__, NULL, "'tree'", git2r_err_tree_arg);
    if (git2r_arg_check_logical(recursive))
        git2r_error(__func__, NULL, "'recursive'", git2r_err_logical_arg);

    repo = GET_SLOT(tree, Rf_install("repo"));
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    sha = GET_SLOT(tree, Rf_install("sha"));
    git_oid_fromstr(&oid, CHAR(STRING_ELT(sha, 0)));
    err = git_tree_lookup(&tree_obj, repository, &oid);
    if (err)
        goto cleanup;

    /* Count number of entries before creating the list */
    cb_data.repository = repository;
    if (LOGICAL(recursive)[0])
        cb_data.recursive = 1;
    err = git_tree_walk(tree_obj, 0, &git2r_tree_walk_cb, &cb_data);
    if (err)
        goto cleanup;

    PROTECT(result = Rf_allocVector(VECSXP, 6));
    nprotect++;
    setAttrib(result, R_NamesSymbol, names = Rf_allocVector(STRSXP, 6));

    SET_VECTOR_ELT(result, 0,   Rf_allocVector(STRSXP,  cb_data.n));
    SET_STRING_ELT(names,  0, mkChar("mode"));
    SET_VECTOR_ELT(result, 1,   Rf_allocVector(STRSXP,  cb_data.n));
    SET_STRING_ELT(names,  1, mkChar("type"));
    SET_VECTOR_ELT(result, 2,   Rf_allocVector(STRSXP,  cb_data.n));
    SET_STRING_ELT(names,  2, mkChar("sha"));
    SET_VECTOR_ELT(result, 3,   Rf_allocVector(STRSXP,  cb_data.n));
    SET_STRING_ELT(names,  3, mkChar("path"));
    SET_VECTOR_ELT(result, 4,   Rf_allocVector(STRSXP,  cb_data.n));
    SET_STRING_ELT(names,  4, mkChar("name"));
    SET_VECTOR_ELT(result, 5,   Rf_allocVector(INTSXP,  cb_data.n));
    SET_STRING_ELT(names,  5, mkChar("len"));

    cb_data.list = result;
    cb_data.n = 0;
    err = git_tree_walk(tree_obj, 0, &git2r_tree_walk_cb, &cb_data);

cleanup:
    if (repository)
        git_repository_free(repository);

    if (tree_obj)
        git_tree_free(tree_obj);

    if (nprotect)
        UNPROTECT(nprotect);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return result;
}
