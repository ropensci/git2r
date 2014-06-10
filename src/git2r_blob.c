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
#include "git2r_blob.h"
#include "git2r_error.h"
#include "git2r_repository.h"

/**
 * Get content of a blob
 *
 * @param blob S4 class git_blob
 * @return content
 */
SEXP git2r_blob_content(SEXP blob)
{
    int err;
    SEXP result = R_NilValue;
    SEXP hex;
    git_blob *blob_obj = NULL;
    git_oid oid;
    git_repository *repository = NULL;

    if (git2r_error_check_blob_arg(blob))
        error("Invalid arguments to git2r_blob_content");

    repository = git2r_repository_open(GET_SLOT(blob, Rf_install("repo")));
    if (!repository)
        error(git2r_err_invalid_repository);

    hex = GET_SLOT(blob, Rf_install("hex"));
    git_oid_fromstr(&oid, CHAR(STRING_ELT(hex, 0)));

    err = git_blob_lookup(&blob_obj, repository, &oid);
    if (err < 0)
        goto cleanup;

    PROTECT(result = allocVector(STRSXP, 1));
    SET_STRING_ELT(result, 0, mkChar(git_blob_rawcontent(blob_obj)));

cleanup:
    if (blob_obj)
        git_blob_free(blob_obj);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return result;
}

/**
 * Read a file from the filesystem and write its content to the
 * Object Database as a loose blob
 * @param repo The repository where the blob will be written. Can be
 * a bare repository.
 * @param path The file from which the blob will be created.
 * @return list of S4 class git_blob objects
 */
SEXP git2r_blob_create_fromdisk(SEXP repo, SEXP path)
{
    SEXP result = R_NilValue;
    SEXP sexp_blob;
    int err;
    size_t len, i;
    git_oid oid;
    git_blob *blob = NULL;
    git_repository *repository = NULL;

    if (R_NilValue == path || !isString(path))
        error("Invalid argument to git2r_blob_create_fromdisk");

    repository = git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    len = length(path);
    PROTECT(result = allocVector(VECSXP, len));
    for (i = 0; i < len; i++) {
        if (NA_STRING != STRING_ELT(path, i)) {
            err = git_blob_create_fromdisk(&oid,
                                           repository,
                                           CHAR(STRING_ELT(path, i)));
            if (err < 0)
                goto cleanup;

            err = git_blob_lookup(&blob, repository, &oid);
            if (err < 0)
                goto cleanup;

            PROTECT(sexp_blob = NEW_OBJECT(MAKE_CLASS("git_blob")));
            git2r_blob_init(blob, repo, sexp_blob);
            SET_VECTOR_ELT(result, i, sexp_blob);
            UNPROTECT(1);
            git_blob_free(blob);
        }
    }

cleanup:
    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return result;
}

/**
 * Init slots in S4 class git_blob
 *
 * @param source a blob
 * @param repo S4 class git_repository that contains the blob
 * @param dest S4 class git_blob to initialize
 * @return void
 */
void git2r_blob_init(const git_blob *source, SEXP repo, SEXP dest)
{
    const git_oid *oid;
    char hex[GIT_OID_HEXSZ + 1];

    oid = git_blob_id(source);
    git_oid_tostr(hex, sizeof(hex), oid);
    SET_SLOT(dest,
             Rf_install("hex"),
             ScalarString(mkChar(hex)));

    SET_SLOT(dest, Rf_install("repo"), duplicate(repo));
}

/**
 * Is blob binary
 *
 * @param blob S4 class git_blob
 * @return TRUE if binary data, FALSE if not
 */
SEXP git2r_blob_is_binary(SEXP blob)
{
    int err;
    SEXP result = R_NilValue;
    SEXP hex;
    git_blob *blob_obj = NULL;
    git_oid oid;
    git_repository *repository = NULL;

    if (git2r_error_check_blob_arg(blob))
        error("Invalid arguments to git2r_blob_is_binary");

    repository= git2r_repository_open(GET_SLOT(blob, Rf_install("repo")));
    if (!repository)
        error(git2r_err_invalid_repository);

    hex = GET_SLOT(blob, Rf_install("hex"));
    git_oid_fromstr(&oid, CHAR(STRING_ELT(hex, 0)));

    err = git_blob_lookup(&blob_obj, repository, &oid);
    if (err < 0)
        goto cleanup;

    PROTECT(result = allocVector(LGLSXP, 1));
    if (git_blob_is_binary(blob_obj))
        LOGICAL(result)[0] = 1;
    else
        LOGICAL(result)[0] = 0;

cleanup:
    if (blob_obj)
        git_blob_free(blob_obj);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return result;
}

/**
 * Size in bytes of contents of a blob
 *
 * @param blob S4 class git_blob
 * @return size
 */
SEXP git2r_blob_rawsize(SEXP blob)
{
    int err;
    SEXP hex;
    git_off_t size;
    git_blob *blob_obj = NULL;
    git_oid oid;
    git_repository *repository = NULL;

    if (git2r_error_check_blob_arg(blob))
        error("Invalid arguments to git2r_blob_rawsize");

    repository= git2r_repository_open(GET_SLOT(blob, Rf_install("repo")));
    if (!repository)
        error(git2r_err_invalid_repository);

    hex = GET_SLOT(blob, Rf_install("hex"));
    git_oid_fromstr(&oid, CHAR(STRING_ELT(hex, 0)));

    err = git_blob_lookup(&blob_obj, repository, &oid);
    if (err < 0)
        goto cleanup;

    size = git_blob_rawsize(blob_obj);

cleanup:
    if (blob_obj)
        git_blob_free(blob_obj);

    if (repository)
        git_repository_free(repository);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return ScalarInteger(size);
}
