/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2014 The git2r contributors
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
#include "git2.h"
#include "buffer.h"

#include "git2r_arg.h"
#include "git2r_error.h"
#include "git2r_odb.h"
#include "git2r_repository.h"

/**
 * Determine the sha of character vectors without writing to the
 * object data base.
 *
 * @param data STRSXP with character vectors to hash
 * @return A STRSXP with character vector of sha values
 */
SEXP git2r_odb_hash(SEXP data)
{
    SEXP result;
    int err = GIT_OK;
    size_t len, i;
    char sha[GIT_OID_HEXSZ + 1];
    git_oid oid;

    if (GIT_OK != git2r_arg_check_string_vec(data))
        git2r_error(git2r_err_string_vec_arg, __func__, "data");

    len = length(data);
    PROTECT(result = allocVector(STRSXP, len));
    for (i = 0; i < len; i++) {
        if (NA_STRING == STRING_ELT(data, i)) {
            SET_STRING_ELT(result, i, NA_STRING);
        } else {
            err = git_odb_hash(&oid,
                               CHAR(STRING_ELT(data, i)),
                               LENGTH(STRING_ELT(data, i)),
                               GIT_OBJ_BLOB);
            if (GIT_OK != err)
                break;

            git_oid_fmt(sha, &oid);
            sha[GIT_OID_HEXSZ] = '\0';
            SET_STRING_ELT(result, i, mkChar(sha));
        }
    }

    UNPROTECT(1);

    if (GIT_OK != err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return result;
}

/**
 * Determine the sha of files without writing to the object data
 * base.
 *
 * @param path STRSXP with file vectors to hash
 * @return A STRSXP with character vector of sha values
 */
SEXP git2r_odb_hashfile(SEXP path)
{
    SEXP result;
    int err = GIT_OK;
    size_t len, i;
    char sha[GIT_OID_HEXSZ + 1];
    git_oid oid;

    if (GIT_OK != git2r_arg_check_string_vec(path))
        git2r_error(git2r_err_string_vec_arg, __func__, "path");

    len = length(path);
    PROTECT(result = allocVector(STRSXP, len));
    for (i = 0; i < len; i++) {
        if (NA_STRING == STRING_ELT(path, i)) {
            SET_STRING_ELT(result, i, NA_STRING);
        } else {
            err = git_odb_hashfile(&oid,
                                   CHAR(STRING_ELT(path, i)),
                                   GIT_OBJ_BLOB);
            if (GIT_OK != err)
                break;

            git_oid_fmt(sha, &oid);
            sha[GIT_OID_HEXSZ] = '\0';
            SET_STRING_ELT(result, i, mkChar(sha));
        }
    }

    UNPROTECT(1);

    if (GIT_OK != err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return result;
}

/**
 * Data structure to hold information when iterating over objects.
 */
typedef struct {
    size_t n;
    SEXP list;
    SEXP repo;
    git_odb *odb;
} git2r_odb_list_cb_data;

/**
 * Add object
 *
 * @param id Oid of the object
 * @param list The list to hold the S4 class git_object
 * @param i The index to the list item
 * @param type The type of the object
 * @param len The length of the object
 * @param repo The S4 class that contains the object
 * @return void
 */
static void git2r_add_object(
    const git_oid *id,
    SEXP list,
    size_t i,
    const char *type,
    size_t len,
    SEXP repo)
{
    char sha[GIT_OID_HEXSZ + 1];
    SEXP object;

    PROTECT(object = NEW_OBJECT(MAKE_CLASS("git_object")));

    git_oid_fmt(sha, id);
    sha[GIT_OID_HEXSZ] = '\0';
    SET_SLOT(object,
             Rf_install("sha"),
             ScalarString(mkChar(sha)));

    SET_SLOT(object,
             Rf_install("type"),
             ScalarString(mkChar(type)));

    SET_SLOT(object,
             Rf_install("len"),
             ScalarInteger(len));

    SET_SLOT(object, Rf_install("repo"), duplicate(repo));

    SET_VECTOR_ELT(list, i, object);
    UNPROTECT(1);
}

/**
 * Callback when iterating over objects
 *
 * @param id Oid of the object
 * @param payload Payload data
 * @return int 0 or error code
 */
static int git2r_odb_list_cb(const git_oid *id, void *payload)
{
    int err;
    size_t len;
    git_otype type;
    git2r_odb_list_cb_data *p = (git2r_odb_list_cb_data*)payload;

    err = git_odb_read_header(&len, &type, p->odb, id);
    if (GIT_OK != err)
        return err;

    switch(type) {
    case GIT_OBJ_COMMIT:
        if (R_NilValue != p->list)
            git2r_add_object(id, p->list, p->n, "commit", len, p->repo);
        break;
    case GIT_OBJ_TREE:
        if (R_NilValue != p->list)
            git2r_add_object(id, p->list, p->n, "tree", len, p->repo);
        break;
    case GIT_OBJ_BLOB:
        if (R_NilValue != p->list)
            git2r_add_object(id, p->list, p->n, "blob", len, p->repo);
        break;
    case GIT_OBJ_TAG:
        if (R_NilValue != p->list)
            git2r_add_object(id, p->list, p->n, "blob", len, p->repo);
        break;
    default:
        return 0;
    }

    p->n += 1;

    return 0;
}

/**
 * List all objects available in the database
 *
 * @param repo S4 class git_repository
 * @return list with sha's for commit's, tree's, blob's and tag's
 */
SEXP git2r_odb_list(SEXP repo)
{
    int err;
    SEXP result = R_NilValue;
    git2r_odb_list_cb_data cb_data = {0, R_NilValue, R_NilValue, NULL};
    git_odb *odb = NULL;
    git_repository *repository = NULL;

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git_repository_odb(&odb, repository);
    if (GIT_OK != err)
        goto cleanup;
    cb_data.odb = odb;

    /* Count number of objects before creating the list */
    err = git_odb_foreach(odb, &git2r_odb_list_cb, &cb_data);
    if (GIT_OK != err)
        goto cleanup;

    PROTECT(result = allocVector(VECSXP, cb_data.n));
    cb_data.list = result;
    cb_data.repo = repo;
    cb_data.n = 0;
    err = git_odb_foreach(odb, &git2r_odb_list_cb, &cb_data);
    if (GIT_OK != err) {
        UNPROTECT(4);
        goto cleanup;
    }

cleanup:
    if (repository)
        git_repository_free(repository);

    if (odb)
        git_odb_free(odb);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (GIT_OK != err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return result;
}

/**
 * Data structure to hold information when iterating over blobs.
 */
typedef struct {
    size_t n;
    SEXP list;
    git_repository *repository;
    git_odb *odb;
} git2r_odb_blobs_cb_data;

/**
 * Add blob entry to list
 *
 * @param entry The tree entry (blob) to add
 * @param odb The object database
 * @param list The list to hold the blob information
 * @param i The vector index of the list items to use for the blob information
 * @param path The path to the tree relative to the repository workdir
 * @param commit The commit that contains the root tree of the iteration
 * @param author The author of the commit
 * @param when Time of the commit
 * @return 0 or error code
 */
static int git2r_odb_add_blob(
    const git_tree_entry *entry,
    git_odb *odb,
    SEXP list,
    size_t i,
    const char *path,
    const char *commit,
    const char *author,
    double when)
{
    int err;
    int j = 0;
    size_t len;
    git_otype type;
    char sha[GIT_OID_HEXSZ + 1];

    /* Sha */
    git_oid_fmt(sha, git_tree_entry_id(entry));
    sha[GIT_OID_HEXSZ] = '\0';
    SET_STRING_ELT(VECTOR_ELT(list, j++), i, mkChar(sha));

    /* Path */
    SET_STRING_ELT(VECTOR_ELT(list, j++), i, mkChar(path));

    /* Name */
    SET_STRING_ELT(VECTOR_ELT(list, j++), i, mkChar(git_tree_entry_name(entry)));

    /* Length */
    err = git_odb_read_header(&len, &type, odb, git_tree_entry_id(entry));
    if (GIT_OK != err)
        return err;
    INTEGER(VECTOR_ELT(list, j++))[i] = len;

    /* Commit sha */
    SET_STRING_ELT(VECTOR_ELT(list, j++), i, mkChar(commit));

    /* Author */
    SET_STRING_ELT(VECTOR_ELT(list, j++), i, mkChar(author));

    /* When */
    REAL(VECTOR_ELT(list, j++))[i] = when;

    return GIT_OK;
}

/**
 * Recursively iterate over all tree's
 *
 * @param tree The tree to iterate over
 * @param path The path to the tree relative to the repository workdir
 * @param commit The commit that contains the root tree of the iteration
 * @param author The author of the commit
 * @param when Time of the commit
 * @param data The callback data when iterating over odb objects
 * @return 0 or error code
 */
static int git2r_odb_tree_blobs(
    const git_tree *tree,
    const char *path,
    const char *commit,
    const char *author,
    double when,
    git2r_odb_blobs_cb_data *data)
{
    int err;
    size_t i, n;

    n = git_tree_entrycount(tree);
    for (i = 0; i < n; ++i) {
        const git_tree_entry *entry;

        entry = git_tree_entry_byindex(tree, i);
        switch (git_tree_entry_type(entry)) {
        case GIT_OBJ_TREE:
        {
            git_buf buf = GIT_BUF_INIT;
            git_tree *sub_tree = NULL;

            err = git_tree_lookup(
                &sub_tree,
                data->repository,
                git_tree_entry_id(entry));
            if (GIT_OK != err)
                return err;

            err = git_buf_joinpath(&buf, path, git_tree_entry_name(entry));

            if (GIT_OK == err) {
                err = git2r_odb_tree_blobs(
                    sub_tree,
                    buf.ptr,
                    commit,
                    author,
                    when,
                    data);
            }

            git_buf_free(&buf);

            if (sub_tree)
                git_tree_free(sub_tree);

            if (GIT_OK != err)
                return err;

            break;
        }
        case GIT_OBJ_BLOB:
            if (R_NilValue != data->list) {
                err = git2r_odb_add_blob(
                    entry,
                    data->odb,
                    data->list,
                    data->n,
                    path,
                    commit,
                    author,
                    when);
                if (GIT_OK != err)
                    return err;
            }
            data->n += 1;
            break;
        default:
            break;
        }
    }

    return GIT_OK;
}

/**
 * Callback when iterating over blobs
 *
 * @param oid Oid of the object
 * @param payload Payload data
 * @return int 0 or error code
 */
static int git2r_odb_blobs_cb(const git_oid *oid, void *payload)
{
    int err = GIT_OK;
    size_t len;
    git_otype type;
    git2r_odb_blobs_cb_data *p = (git2r_odb_blobs_cb_data*)payload;

    err = git_odb_read_header(&len, &type, p->odb, oid);
    if (GIT_OK != err)
        return err;

    if (GIT_OBJ_COMMIT == type) {
        const git_signature *author;
        git_commit *commit = NULL;
        git_tree *tree = NULL;
        char sha[GIT_OID_HEXSZ + 1];

        err = git_commit_lookup(&commit, p->repository, oid);
        if (GIT_OK != err)
            goto cleanup;

        err = git_commit_tree(&tree, commit);
        if (GIT_OK != err)
            goto cleanup;

        git_oid_fmt(sha, oid);
        sha[GIT_OID_HEXSZ] = '\0';

        author = git_commit_author(commit);

        /* Recursively iterate over all tree's */
        err = git2r_odb_tree_blobs(
            tree,
            "",
            sha,
            author->name,
            (double)(author->when.time) + 60 * (double)(author->when.offset),
            p);

    cleanup:
        if (commit)
            git_commit_free(commit);

        if (tree)
            git_tree_free(tree);
    }

    return err;
}

/**
 * List all blobs available in the database
 *
 * List all blobs reachable from the commits in the object
 * database. First list all commits. Then iterate over each blob from
 * the tree and sub-trees of each commit.
 * @param repo S4 class git_repository
 * @return A list with blob entries
 */
SEXP git2r_odb_blobs(SEXP repo)
{
    int i, err;
    SEXP result = R_NilValue;
    SEXP names = R_NilValue;
    git2r_odb_blobs_cb_data cb_data = {0, R_NilValue, NULL, NULL};
    git_odb *odb = NULL;
    git_repository *repository = NULL;

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git_repository_odb(&odb, repository);
    if (GIT_OK != err)
        goto cleanup;
    cb_data.odb = odb;

    /* Count number of blobs before creating the list */
    cb_data.repository = repository;
    err = git_odb_foreach(odb, &git2r_odb_blobs_cb, &cb_data);
    if (GIT_OK != err)
        goto cleanup;

    PROTECT(result = allocVector(VECSXP, 7));
    PROTECT(names = allocVector(STRSXP, 7));
    i = 0;
    SET_VECTOR_ELT(result, i,   allocVector(STRSXP,  cb_data.n));
    SET_STRING_ELT(names,  i++, mkChar("sha"));
    SET_VECTOR_ELT(result, i,   allocVector(STRSXP,  cb_data.n));
    SET_STRING_ELT(names,  i++, mkChar("path"));
    SET_VECTOR_ELT(result, i,   allocVector(STRSXP,  cb_data.n));
    SET_STRING_ELT(names,  i++, mkChar("name"));
    SET_VECTOR_ELT(result, i,   allocVector(INTSXP,  cb_data.n));
    SET_STRING_ELT(names,  i++, mkChar("len"));
    SET_VECTOR_ELT(result, i,   allocVector(STRSXP,  cb_data.n));
    SET_STRING_ELT(names,  i++, mkChar("commit"));
    SET_VECTOR_ELT(result, i,   allocVector(STRSXP,  cb_data.n));
    SET_STRING_ELT(names,  i++, mkChar("author"));
    SET_VECTOR_ELT(result, i,   allocVector(REALSXP, cb_data.n));
    SET_STRING_ELT(names,  i++, mkChar("when"));
    setAttrib(result, R_NamesSymbol, names);
    UNPROTECT(1);

    cb_data.list = result;
    cb_data.n = 0;
    err = git_odb_foreach(odb, &git2r_odb_blobs_cb, &cb_data);

cleanup:
    if (repository)
        git_repository_free(repository);

    if (odb)
        git_odb_free(odb);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (GIT_OK != err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return result;
}
