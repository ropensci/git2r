/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2017 The git2r contributors
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
#include "git2r_reference.h"
#include "git2r_repository.h"

/**
 * Lookup the full name of a reference by DWIMing its short name
 *
 * @param repo S4 class git_repository
 * @param shorthand The short name for the reference
 * @return Character vector of length one with the full name of the
 * reference
 */
SEXP git2r_reference_dwim(SEXP repo, SEXP shorthand)
{
    int err;
    SEXP result = R_NilValue;
    git_reference* reference = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_string(shorthand))
        git2r_error(__func__, NULL, "'shorthand'", git2r_err_string_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    err = git_reference_dwim(
        &reference,
        repository,
        CHAR(STRING_ELT(shorthand, 0)));
    if (err)
        goto cleanup;

    PROTECT(result = Rf_allocVector(STRSXP, 1));
    SET_STRING_ELT(result, 0, mkChar(git_reference_name(reference)));

cleanup:
    if (reference)
        git_reference_free(reference);

    if (repository)
        git_repository_free(repository);

    if (!Rf_isNull(result))
        UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return result;
}

/**
 * Init slots in S4 class git_reference.
 *
 * @param source A git_reference pointer
 * @param dest S4 class git_reference to initialize
 * @return void
 */
void git2r_reference_init(git_reference *source, SEXP dest)
{
    char sha[GIT_OID_HEXSZ + 1];
    SEXP s_name = Rf_install("name");
    SEXP s_shorthand = Rf_install("shorthand");
    SEXP s_type = Rf_install("type");
    SEXP s_sha = Rf_install("sha");
    SEXP s_target = Rf_install("target");

    SET_SLOT(dest, s_name, mkString(git_reference_name(source)));
    SET_SLOT(dest, s_shorthand, mkString(git_reference_shorthand(source)));

    switch (git_reference_type(source)) {
    case GIT_REF_OID:
        SET_SLOT(dest, s_type, ScalarInteger(GIT_REF_OID));
        git_oid_fmt(sha, git_reference_target(source));
        sha[GIT_OID_HEXSZ] = '\0';
        SET_SLOT(dest, s_sha, mkString(sha));
        break;
    case GIT_REF_SYMBOLIC:
        SET_SLOT(dest, s_type, ScalarInteger(GIT_REF_SYMBOLIC));
        SET_SLOT(dest, s_target, mkString(git_reference_symbolic_target(source)));
        break;
    default:
        git2r_error(__func__, NULL, git2r_err_reference, NULL);
    }
}

/**
 * Get all references that can be found in a repository.
 *
 * @param repo S4 class git_repository
 * @return VECXSP with S4 objects of class git_reference
 */
SEXP git2r_reference_list(SEXP repo)
{
    int err;
    size_t i;
    git_strarray ref_list;
    SEXP result = R_NilValue;
    SEXP names = R_NilValue;
    git_repository *repository = NULL;

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    err = git_reference_list(&ref_list, repository);
    if (err)
        goto cleanup;

    PROTECT(result = Rf_allocVector(VECSXP, ref_list.count));
    setAttrib(
        result,
        R_NamesSymbol,
        names = Rf_allocVector(STRSXP, ref_list.count));

    for (i = 0; i < ref_list.count; i++) {
        SEXP reference;
        git_reference *ref = NULL;

        err = git_reference_lookup(&ref, repository, ref_list.strings[i]);
        if (err)
            goto cleanup;

        SET_VECTOR_ELT(
            result,
            i,
            reference = NEW_OBJECT(MAKE_CLASS("git_reference")));
        git2r_reference_init(ref, reference);
        SET_STRING_ELT(names, i, mkChar(ref_list.strings[i]));

        if (ref)
            git_reference_free(ref);
    }

cleanup:
    git_strarray_free(&ref_list);

    if (repository)
        git_repository_free(repository);

    if (!Rf_isNull(result))
        UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return result;
}
