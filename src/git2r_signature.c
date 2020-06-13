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
#include "git2r_error.h"
#include "git2r_deprecated.h"
#include "git2r_repository.h"
#include "git2r_S3.h"
#include "git2r_signature.h"

/**
 * Get the configured signature for a repository
 *
 * @param repo S3 class git_repository
 * @return S3 class git_signature
 */
SEXP attribute_hidden
git2r_signature_default(
    SEXP repo)
{
    int error, nprotect = 0;
    git_repository *repository = NULL;
    git_signature *signature = NULL;
    SEXP result = R_NilValue;

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git_signature_default(&signature, repository);
    if (error)
        goto cleanup;

    PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_signature));
    nprotect++;
    Rf_setAttrib(result, R_ClassSymbol,
                 Rf_mkString(git2r_S3_class__git_signature));
    git2r_signature_init(signature, result);

cleanup:
    git_repository_free(repository);
    git_signature_free(signature);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Init signature from SEXP signature argument
 *
 * @param out new signature
 * @param signature SEXP argument with S3 class git_signature
 * @return 0 or an error code
 */
int attribute_hidden
git2r_signature_from_arg(
    git_signature **out,
    SEXP signature)
{
    SEXP when = git2r_get_list_element(signature, "when");

    return git_signature_new(
        out,
        CHAR(STRING_ELT(git2r_get_list_element(signature, "name"), 0)),
        CHAR(STRING_ELT(git2r_get_list_element(signature, "email"), 0)),
        REAL(git2r_get_list_element(when, "time"))[0],
        REAL(git2r_get_list_element(when, "offset"))[0]);
}

/**
 * Init slots in S3 class git_signature.
 *
 * @param source A git signature
 * @param dest S3 class git_signature to initialize
 * @return void
 */
void attribute_hidden
git2r_signature_init(
    const git_signature *source,
    SEXP dest)
{
    SEXP when;

    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_signature__name,
        Rf_mkString(source->name));

    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_signature__email,
        Rf_mkString(source->email));

    when = VECTOR_ELT(dest, git2r_S3_item__git_signature__when);
    if (Rf_isNull(when)) {
        SET_VECTOR_ELT(
            dest,
            git2r_S3_item__git_signature__when,
            when = Rf_mkNamed(VECSXP, git2r_S3_items__git_time));
        Rf_setAttrib(when, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_time));
    }

    SET_VECTOR_ELT(
        when,
        git2r_S3_item__git_time__time,
        Rf_ScalarReal((double)source->when.time));

    SET_VECTOR_ELT(
        when,
        git2r_S3_item__git_time__offset,
        Rf_ScalarReal((double)source->when.offset));
}
