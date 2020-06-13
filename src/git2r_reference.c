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
#include "git2r_reference.h"
#include "git2r_repository.h"
#include "git2r_S3.h"

/**
 * Init slots in S3 class git_reference.
 *
 * @param source A git_reference pointer
 * @param repo S3 class git_repository that contains the reference
 * @param dest S3 class git_reference to initialize
 * @return void
 */
static void
git2r_reference_init(
    git_reference *source,
    SEXP repo,
    SEXP dest)
{
    char sha[GIT_OID_HEXSZ + 1];

    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_reference__name,
        Rf_mkString(git_reference_name(source)));

    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_reference__shorthand,
        Rf_mkString(git_reference_shorthand(source)));

    switch (git_reference_type(source)) {
    case GIT2R_REFERENCE_DIRECT:
        SET_VECTOR_ELT(
            dest,
            git2r_S3_item__git_reference__type,
            Rf_ScalarInteger(GIT2R_REFERENCE_DIRECT));

        git_oid_fmt(sha, git_reference_target(source));
        sha[GIT_OID_HEXSZ] = '\0';
        SET_VECTOR_ELT(
            dest,
            git2r_S3_item__git_reference__sha,
            Rf_mkString(sha));
        break;
    case GIT2R_REFERENCE_SYMBOLIC:
        SET_VECTOR_ELT(
            dest,
            git2r_S3_item__git_reference__type,
            Rf_ScalarInteger(GIT2R_REFERENCE_SYMBOLIC));

        SET_VECTOR_ELT(
            dest,
            git2r_S3_item__git_reference__target,
            Rf_mkString(git_reference_symbolic_target(source)));
        break;
    default:
        git2r_error(__func__, NULL, git2r_err_reference, NULL);
    }

    if (Rf_isNull(VECTOR_ELT(dest, git2r_S3_item__git_reference__target))) {
        SET_VECTOR_ELT(
            dest,
            git2r_S3_item__git_reference__target,
            Rf_ScalarString(NA_STRING));
    }

    SET_VECTOR_ELT(dest, git2r_S3_item__git_reference__repo, Rf_duplicate(repo));
}

/**
 * Lookup the full name of a reference by DWIMing its short name
 *
 * @param repo S3 class git_repository
 * @param shorthand The short name for the reference
 * @return S3 class git_reference object
 */
SEXP attribute_hidden
git2r_reference_dwim(
    SEXP repo,
    SEXP shorthand)
{
    int error, nprotect = 0;
    SEXP result = R_NilValue;
    git_reference* reference = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_string(shorthand))
        git2r_error(__func__, NULL, "'shorthand'", git2r_err_string_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git_reference_dwim(
        &reference,
        repository,
        CHAR(STRING_ELT(shorthand, 0)));
    if (error)
        goto cleanup;

    PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_reference));
    nprotect++;
    Rf_setAttrib(result, R_ClassSymbol,
                 Rf_mkString(git2r_S3_class__git_reference));
    git2r_reference_init(reference, repo, result);

cleanup:
    git_reference_free(reference);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Get all references that can be found in a repository.
 *
 * @param repo S3 class git_repository
 * @return VECXSP with S3 objects of class git_reference
 */
SEXP attribute_hidden
git2r_reference_list(
    SEXP repo)
{
    int error, nprotect = 0;
    size_t i;
    git_strarray ref_list;
    SEXP result = R_NilValue;
    SEXP names = R_NilValue;
    git_repository *repository = NULL;

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git_reference_list(&ref_list, repository);
    if (error)
        goto cleanup;

    PROTECT(result = Rf_allocVector(VECSXP, ref_list.count));
    nprotect++;
    Rf_setAttrib(
        result,
        R_NamesSymbol,
        names = Rf_allocVector(STRSXP, ref_list.count));

    for (i = 0; i < ref_list.count; i++) {
        SEXP reference;
        git_reference *ref = NULL;

        error = git_reference_lookup(&ref, repository, ref_list.strings[i]);
        if (error)
            goto cleanup;

        SET_VECTOR_ELT(
            result,
            i,
            reference = Rf_mkNamed(VECSXP, git2r_S3_items__git_reference));
        Rf_setAttrib(reference, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_reference));
        git2r_reference_init(ref, repo, reference);
        SET_STRING_ELT(names, i, Rf_mkChar(ref_list.strings[i]));

        git_reference_free(ref);
    }

cleanup:
    git_strarray_free(&ref_list);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}
