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
    SEXP result = R_NilValue;

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    err = git_signature_default(&signature, repository);
    if (err)
        goto cleanup;

    PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_signature")));
    git2r_signature_init(signature, result);

cleanup:
    if (repository)
        git_repository_free(repository);

    if (signature)
        git_signature_free(signature);

    if (!Rf_isNull(result))
        UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return result;
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
    err = git_signature_new(
        out,
        CHAR(STRING_ELT(GET_SLOT(signature, Rf_install("name")), 0)),
        CHAR(STRING_ELT(GET_SLOT(signature, Rf_install("email")), 0)),
        REAL(GET_SLOT(when, Rf_install("time")))[0],
        REAL(GET_SLOT(when, Rf_install("offset")))[0]);

    return err;
}

/**
 * Init slots in S4 class git_signature.
 *
 * @param source A git signature
 * @param dest S4 class git_signature to initialize
 * @return void
 */
void git2r_signature_init(const git_signature *source, SEXP dest)
{
    SEXP when;
    SEXP s_name = Rf_install("name");
    SEXP s_email = Rf_install("email");
    SEXP s_time = Rf_install("time");
    SEXP s_offset = Rf_install("offset");

    SET_SLOT(dest, s_name, Rf_mkString(source->name));
    SET_SLOT(dest, s_email, Rf_mkString(source->email));

    when = GET_SLOT(dest, Rf_install("when"));
    SET_SLOT(when, s_time, ScalarReal((double)source->when.time));
    SET_SLOT(when, s_offset, ScalarReal((double)source->when.offset));
}
