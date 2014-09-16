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
 * Callback when iterating over objects
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
