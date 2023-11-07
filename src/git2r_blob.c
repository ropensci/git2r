/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2023 The git2r contributors
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
#include "git2r_arg.h"
#include "git2r_blob.h"
#include "git2r_deprecated.h"
#include "git2r_error.h"
#include "git2r_repository.h"
#include "git2r_S3.h"

/**
 * Get content of a blob
 *
 * @param blob S3 class git_blob
 * @param raw If true, return content as a RAWSXP vector, else as a
 * STRSXP vector.
 * @return content
 */
SEXP attribute_hidden
git2r_blob_content(
    SEXP blob,
    SEXP raw)
{
    int error, nprotect = 0;
    SEXP result = R_NilValue;
    SEXP sha;
    git_blob *blob_obj = NULL;
    git_oid oid;
    git_repository *repository = NULL;

    if (git2r_arg_check_blob(blob))
        git2r_error(__func__, NULL, "'blob'", git2r_err_blob_arg);
    if (git2r_arg_check_logical(raw))
        git2r_error(__func__, NULL, "'raw'", git2r_err_logical_arg);

    repository = git2r_repository_open(git2r_get_list_element(blob, "repo"));
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    sha = git2r_get_list_element(blob, "sha");
    git_oid_fromstr(&oid, CHAR(STRING_ELT(sha, 0)));

    error = git_blob_lookup(&blob_obj, repository, &oid);
    if (error)
        goto cleanup;

    if (LOGICAL(raw)[0]) {
        PROTECT(result = Rf_allocVector(RAWSXP, git_blob_rawsize(blob_obj)));
        nprotect++;
        memcpy(
            RAW(result),
            git_blob_rawcontent(blob_obj),
            git_blob_rawsize(blob_obj));
    } else {
        PROTECT(result = Rf_allocVector(STRSXP, 1));
        nprotect++;
        if (git_blob_is_binary(blob_obj)) {
            SET_STRING_ELT(result, 0, NA_STRING);
        } else {
            SET_STRING_ELT(result, 0, Rf_mkChar(git_blob_rawcontent(blob_obj)));
        }
    }

cleanup:
    git_blob_free(blob_obj);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Read a file from the filesystem and write its content to the
 * Object Database as a loose blob
 * @param repo The repository where the blob will be written. Can be
 * a bare repository.
 * @param path The file from which the blob will be created.
 * @return list of S3 class git_blob objects
 */
SEXP attribute_hidden
git2r_blob_create_fromdisk(
    SEXP repo,
    SEXP path)
{
    SEXP result = R_NilValue;
    int error = 0, nprotect = 0;
    size_t len, i;
    git_repository *repository = NULL;

    if (git2r_arg_check_string_vec(path))
        git2r_error(__func__, NULL, "'path'", git2r_err_string_vec_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    len = Rf_length(path);
    PROTECT(result = Rf_allocVector(VECSXP, len));
    nprotect++;
    for (i = 0; i < len; i++) {
        if (NA_STRING != STRING_ELT(path, i)) {
            git_oid oid;
            git_blob *blob = NULL;
            SEXP item;

            error = GIT2R_BLOB_CREATE_FROM_DISK(
                &oid,
                repository,
                CHAR(STRING_ELT(path, i)));
            if (error)
                goto cleanup;

            error = git_blob_lookup(&blob, repository, &oid);
            if (error)
                goto cleanup;

            SET_VECTOR_ELT(result, i,
                           item = Rf_mkNamed(VECSXP, git2r_S3_items__git_blob));
            Rf_setAttrib(item, R_ClassSymbol,
                         Rf_mkString(git2r_S3_class__git_blob));
            git2r_blob_init(blob, repo, item);
            git_blob_free(blob);
        }
    }

cleanup:
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Create blob from file in working directory
 *
 * Read a file from the working folder of a repository and write its
 * content to the Object Database as a loose blob. The method is
 * vectorized and accepts a vector of files to create blobs from.
 * @param repo The repository where the blob(s) will be
 * written. Cannot be a bare repository.
 * @param relative_path The file(s) from which the blob will be
 * created, relative to the repository's working dir.
 * @return list of S3 class git_blob objects
 */
SEXP attribute_hidden
git2r_blob_create_fromworkdir(
    SEXP repo,
    SEXP relative_path)
{
    SEXP result = R_NilValue;
    int error = 0, nprotect = 0;
    size_t len, i;
    git_repository *repository = NULL;

    if (git2r_arg_check_string_vec(relative_path))
        git2r_error(__func__, NULL, "'relative_path'", git2r_err_string_vec_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    len = Rf_length(relative_path);
    PROTECT(result = Rf_allocVector(VECSXP, len));
    nprotect++;
    for (i = 0; i < len; i++) {
        if (NA_STRING != STRING_ELT(relative_path, i)) {
            git_oid oid;
            git_blob *blob = NULL;
            SEXP item;

            error = GIT2R_BLOB_CREATE_FROM_WORKDIR(
                &oid,
                repository,
                CHAR(STRING_ELT(relative_path, i)));
            if (error)
                goto cleanup;

            error = git_blob_lookup(&blob, repository, &oid);
            if (error)
                goto cleanup;

            SET_VECTOR_ELT(result, i,
                           item = Rf_mkNamed(VECSXP, git2r_S3_items__git_blob));
            Rf_setAttrib(item, R_ClassSymbol,
                         Rf_mkString(git2r_S3_class__git_blob));
            git2r_blob_init(blob, repo, item);
            git_blob_free(blob);
        }
    }

cleanup:
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Init entries in a S3 class git_blob
 *
 * @param source a blob
 * @param repo S3 class git_repository that contains the blob
 * @param dest S3 class git_blob to initialize
 * @return void
 */
void attribute_hidden
git2r_blob_init(
    const git_blob *source,
    SEXP repo,
    SEXP dest)
{
    const git_oid *oid;
    char sha[GIT_OID_HEXSZ + 1];

    oid = git_blob_id(source);
    git_oid_tostr(sha, sizeof(sha), oid);
    SET_VECTOR_ELT(dest, git2r_S3_item__git_blob__sha, Rf_mkString(sha));
    SET_VECTOR_ELT(dest, git2r_S3_item__git_blob__repo, Rf_duplicate(repo));
}

/**
 * Is blob binary
 *
 * @param blob S3 class git_blob
 * @return TRUE if binary data, FALSE if not
 */
SEXP attribute_hidden
git2r_blob_is_binary(
    SEXP blob)
{
    int error, nprotect = 0;
    SEXP result = R_NilValue;
    SEXP sha;
    git_blob *blob_obj = NULL;
    git_oid oid;
    git_repository *repository = NULL;

    if (git2r_arg_check_blob(blob))
        git2r_error(__func__, NULL, "'blob'", git2r_err_blob_arg);

    repository = git2r_repository_open(git2r_get_list_element(blob, "repo"));
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    sha = git2r_get_list_element(blob, "sha");
    git_oid_fromstr(&oid, CHAR(STRING_ELT(sha, 0)));

    error = git_blob_lookup(&blob_obj, repository, &oid);
    if (error)
        goto cleanup;

    PROTECT(result = Rf_allocVector(LGLSXP, 1));
    nprotect++;
    if (git_blob_is_binary(blob_obj))
        LOGICAL(result)[0] = 1;
    else
        LOGICAL(result)[0] = 0;

cleanup:
    git_blob_free(blob_obj);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Size in bytes of contents of a blob
 *
 * @param blob S3 class git_blob
 * @return size
 */
SEXP attribute_hidden
git2r_blob_rawsize(
    SEXP blob)
{
    int error;
    SEXP sha;
    git_off_t size = 0;
    git_blob *blob_obj = NULL;
    git_oid oid;
    git_repository *repository = NULL;

    if (git2r_arg_check_blob(blob))
        git2r_error(__func__, NULL, "'blob'", git2r_err_blob_arg);

    repository = git2r_repository_open(git2r_get_list_element(blob, "repo"));
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    sha = git2r_get_list_element(blob, "sha");
    git_oid_fromstr(&oid, CHAR(STRING_ELT(sha, 0)));

    error = git_blob_lookup(&blob_obj, repository, &oid);
    if (error)
        goto cleanup;

    size = git_blob_rawsize(blob_obj);

cleanup:
    git_blob_free(blob_obj);
    git_repository_free(repository);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return Rf_ScalarInteger(size);
}
