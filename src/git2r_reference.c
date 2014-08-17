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
#include "git2r_error.h"
#include "git2r_reference.h"
#include "git2r_repository.h"

/**
 * Init slots in S4 class git_reference.
 *
 * @param source A git_reference pointer
 * @param dest S4 class git_reference to initialize
 * @return void
 */
void git2r_reference_init(git_reference *source, SEXP dest)
{
    char hex[GIT_OID_HEXSZ + 1];

    SET_SLOT(dest,
             Rf_install("name"),
             ScalarString(mkChar(git_reference_name(source))));

    SET_SLOT(dest,
             Rf_install("shorthand"),
             ScalarString(mkChar(git_reference_shorthand(source))));

    switch (git_reference_type(source)) {
    case GIT_REF_OID:
        SET_SLOT(dest, Rf_install("type"), ScalarInteger(GIT_REF_OID));
        git_oid_fmt(hex, git_reference_target(source));
        hex[GIT_OID_HEXSZ] = '\0';
        SET_SLOT(dest, Rf_install("hex"), ScalarString(mkChar(hex)));
        break;
    case GIT_REF_SYMBOLIC:
        SET_SLOT(dest, Rf_install("type"), ScalarInteger(GIT_REF_SYMBOLIC));
        SET_SLOT(dest,
                 Rf_install("target"),
                 ScalarString(mkChar(git_reference_symbolic_target(source))));
        break;
    default:
        git2r_error("Error in '%s': Unexpected reference type", __func__, NULL);
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
    SEXP list = R_NilValue;
    SEXP names = R_NilValue;
    git_reference *ref;
    git_repository *repository;

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git_reference_list(&ref_list, repository);
    if (GIT_OK != err)
        goto cleanup;

    PROTECT(list = allocVector(VECSXP, ref_list.count));
    PROTECT(names = allocVector(STRSXP, ref_list.count));

    for (i = 0; i < ref_list.count; i++) {
        SEXP reference;

        err = git_reference_lookup(&ref, repository, ref_list.strings[i]);
        if (GIT_OK != err)
            goto cleanup;

        PROTECT(reference = NEW_OBJECT(MAKE_CLASS("git_reference")));
        git2r_reference_init(ref, reference);
        SET_STRING_ELT(names, i, mkChar(ref_list.strings[i]));
        SET_VECTOR_ELT(list, i, reference);
        UNPROTECT(1);
    }

cleanup:
    git_strarray_free(&ref_list);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != list && R_NilValue != names) {
        setAttrib(list, R_NamesSymbol, names);
        UNPROTECT(2);
    }

    if (GIT_OK != err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return list;
}
