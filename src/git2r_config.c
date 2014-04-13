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
#include "git2r_repository.h"

/**
 * Config
 *
 * @param repo S4 class git_repository
 * @param variables
 * @return R_NilValue
 */
SEXP config(SEXP repo, SEXP variables)
{
    SEXP names;
    int err, i;
    git_config *cfg = NULL;
    git_repository *repository = NULL;

    if (R_NilValue == variables)
        error("'variables' equals R_NilValue.");
    if (!isNewList(variables))
        error("'variables' must be a list.");

    repository = get_repository(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    err = git_repository_config(&cfg, repository);
    if (err < 0)
        goto cleanup;

    names = getAttrib(variables, R_NamesSymbol);
    for (i = 0; i < length(variables); i++) {
        const char *key = CHAR(STRING_ELT(names, i));
        const char *value = CHAR(STRING_ELT(VECTOR_ELT(variables, i), 0));

        err = git_config_set_string(cfg, key, value);
        if (err < 0)
            goto cleanup;
    }

cleanup:
    if (config)
        git_config_free(cfg);

    if (repository)
        git_repository_free(repository);

    return R_NilValue;
}
