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
#include "git2r_note.h"
#include "git2r_repository.h"
#include "git2.h"

/**
 * Default notes reference
 *
 * Get the default notes reference for a repository
 * @param repo S4 class git_repository
 * @return Character vector of length one with name of default
 * reference
*/
SEXP git2r_note_default_ref(SEXP repo)
{
    int err;
    SEXP result = R_NilValue;
    const char *ref;
    git_repository *repository = NULL;

    repository = git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    err = git_note_default_ref(&ref, repository);
    if (err < 0)
        goto cleanup;

    PROTECT(result = allocVector(STRSXP, 1));
    SET_STRING_ELT(result, 0, mkChar(ref));

cleanup:
    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return result;
}
