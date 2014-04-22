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
SEXP default_signature(SEXP repo)
{
    int err;
    git_repository *repository = NULL;
    git_signature *signature = NULL;
    SEXP sig;

    repository = get_repository(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    err = git_signature_default(&signature, repository);
    if (err < 0)
        goto cleanup;

    PROTECT(sig = NEW_OBJECT(MAKE_CLASS("git_signature")));
    init_signature(signature, sig);

cleanup:
    if (repository)
        git_repository_free(repository);

    if (signature)
        git_signature_free(signature);

    UNPROTECT(1);

    if (err < 0) {
        const git_error *e = giterr_last();
        error("Error %d/%d: %s\n", err, e->klass, e->message);
    }

    return sig;
}

/**
 * Init slots in S4 class git_signature.
 *
 * @param sig
 * @param signature
 * @return void
 */
void init_signature(const git_signature *sig, SEXP signature)
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
