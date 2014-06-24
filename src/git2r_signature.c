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
#include "git2r_error.h"
#include "git2r_repository.h"
#include "git2r_signature.h"

/**
 * Get the configured signature for a repository
 *
 * @param repo S4 class git_repository
 * @return S4 class git_signature
 */
SEXP git2r_signature_default(SEXP repo)
{
    int err;
    git_repository *repository = NULL;
    git_signature *signature = NULL;
    SEXP sig;

    repository = git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    err = git_signature_default(&signature, repository);
    if (err < 0)
        goto cleanup;

    PROTECT(sig = NEW_OBJECT(MAKE_CLASS("git_signature")));
    git2r_signature_init(signature, sig);

cleanup:
    if (repository)
        git_repository_free(repository);

    if (signature)
        git_signature_free(signature);

    UNPROTECT(1);

    if (err < 0)
        error("Error %d/%d: %s\n", giterr_last()->message);

    return sig;
}

/**
 * Init signature from SEXP signature argument
 *
 * @param out new signature
 * @param signature SEXP argument with S4 class git_signature
 * @return void
 */
int git2r_signature_from_arg(git_signature **out, SEXP signature)
{
    int err;
    SEXP when;

    when = GET_SLOT(signature, Rf_install("when"));
    err = git_signature_new(out,
                            CHAR(STRING_ELT(GET_SLOT(signature, Rf_install("name")), 0)),
                            CHAR(STRING_ELT(GET_SLOT(signature, Rf_install("email")), 0)),
                            REAL(GET_SLOT(when, Rf_install("time")))[0],
                            REAL(GET_SLOT(when, Rf_install("offset")))[0]);

    return err;
}

/**
 * Init slots in S4 class git_signature.
 *
 * @param sig
 * @param signature
 * @return void
 */
void git2r_signature_init(const git_signature *sig, SEXP signature)
{
    SEXP when;

    SET_SLOT(signature,
             Rf_install("name"),
             ScalarString(mkChar(sig->name)));

    SET_SLOT(signature,
             Rf_install("email"),
             ScalarString(mkChar(sig->email)));

    when = GET_SLOT(signature, Rf_install("when"));

    SET_SLOT(when,
             Rf_install("time"),
             ScalarReal((double)sig->when.time));

    SET_SLOT(when,
             Rf_install("offset"),
             ScalarReal((double)sig->when.offset));
}
