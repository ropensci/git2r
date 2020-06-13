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
#include "git2r_blame.h"
#include "git2r_deprecated.h"
#include "git2r_error.h"
#include "git2r_repository.h"
#include "git2r_S3.h"
#include "git2r_signature.h"

/**
 * Init slots in S3 class git_blame
 *
 * Iterates over and init each git_blame_hunk
 * @param source a blame object
 * @param repo S3 class git_repository that contains the blame
 * @param path The path to the file to get the blame
 * @param dest S3 class git_blame to initialize
 * @return void
 */
void static
git2r_blame_init(
    git_blame *source,
    SEXP repo,
    SEXP path,
    SEXP dest)
{
    SEXP hunks;
    size_t i, n;

    n = git_blame_get_hunk_count(source);
    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_blame__hunks,
        hunks = Rf_allocVector(VECSXP, n));

    for (i = 0; i < n; i++) {
        const git_blame_hunk *hunk;

        hunk = git_blame_get_hunk_byindex(source, i);
        if (hunk) {
            SEXP item, signature;
            char sha[GIT_OID_HEXSZ + 1];

            SET_VECTOR_ELT(
                hunks,
                i,
                item = Rf_mkNamed(VECSXP, git2r_S3_items__git_blame_hunk));
            Rf_setAttrib(
                item,
                R_ClassSymbol,
                Rf_mkString(git2r_S3_class__git_blame_hunk));

            SET_VECTOR_ELT(
                item,
                git2r_S3_item__git_blame_hunk__lines_in_hunk,
                Rf_ScalarInteger(hunk->lines_in_hunk));

            git_oid_fmt(sha, &(hunk->final_commit_id));
            sha[GIT_OID_HEXSZ] = '\0';
            SET_VECTOR_ELT(
                item,
                git2r_S3_item__git_blame_hunk__final_commit_id,
                Rf_mkString(sha));

            SET_VECTOR_ELT(
                item,
                git2r_S3_item__git_blame_hunk__final_start_line_number,
                Rf_ScalarInteger(hunk->final_start_line_number));

            SET_VECTOR_ELT(
                item,
                git2r_S3_item__git_blame_hunk__final_signature,
                signature = Rf_mkNamed(VECSXP, git2r_S3_items__git_signature));
            Rf_setAttrib(
                signature,
                R_ClassSymbol,
                Rf_mkString(git2r_S3_class__git_signature));
            git2r_signature_init(hunk->final_signature, signature);

            git_oid_fmt(sha, &(hunk->orig_commit_id));
            sha[GIT_OID_HEXSZ] = '\0';
            SET_VECTOR_ELT(
                item,
                git2r_S3_item__git_blame_hunk__orig_commit_id,
                Rf_mkString(sha));

            SET_VECTOR_ELT(
                item,
                git2r_S3_item__git_blame_hunk__orig_start_line_number,
                Rf_ScalarInteger(hunk->orig_start_line_number));

            SET_VECTOR_ELT(
                item,
                git2r_S3_item__git_blame_hunk__orig_signature,
                signature = Rf_mkNamed(VECSXP, git2r_S3_items__git_signature));
            Rf_setAttrib(signature, R_ClassSymbol,
                         Rf_mkString(git2r_S3_class__git_signature));
            git2r_signature_init(hunk->orig_signature, signature);

            SET_VECTOR_ELT(
                item,
                git2r_S3_item__git_blame_hunk__orig_path,
                Rf_mkString(hunk->orig_path));

            if (hunk->boundary) {
                SET_VECTOR_ELT(
                    item,
                    git2r_S3_item__git_blame_hunk__boundary,
                    Rf_ScalarLogical(1));
            } else {
                SET_VECTOR_ELT(
                    item,
                    git2r_S3_item__git_blame_hunk__boundary,
                    Rf_ScalarLogical(0));
            }

            SET_VECTOR_ELT(
                item,
                git2r_S3_item__git_blame_hunk__repo,
                Rf_duplicate(repo));
        }
    }

    SET_VECTOR_ELT(dest, git2r_S3_item__git_blame__path, path);
    SET_VECTOR_ELT(dest, git2r_S3_item__git_blame__repo, Rf_duplicate(repo));
}

/**
 * Get the blame for a single file
 *
 * @param repo S3 class git_repository that contains the blob
 * @param path The path to the file to get the blame
 * @return S3 class git_blame
 */
SEXP attribute_hidden
git2r_blame_file(
    SEXP repo,
    SEXP path)
{
    int error, nprotect = 0;
    SEXP result = R_NilValue;
    git_blame *blame = NULL;
    git_repository *repository = NULL;
    git_blame_options blame_opts = GIT_BLAME_OPTIONS_INIT;

    if (git2r_arg_check_string(path))
        git2r_error(__func__, NULL, "'path'", git2r_err_string_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git_blame_file(
        &blame,
        repository,
        CHAR(STRING_ELT(path, 0)),
        &blame_opts);
    if (error)
        goto cleanup;

    PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_blame));
    nprotect++;
    Rf_setAttrib(result, R_ClassSymbol,
                 Rf_mkString(git2r_S3_class__git_blame));
    git2r_blame_init(blame, repo, path, result);

cleanup:
    git_blame_free(blame);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}
