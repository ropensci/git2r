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
 * Count number of config variables
 *
 * @param cfg where to count the variables
 * @param n pointer to store the number of variables
 * @param 0 on succes, or error code
 */
static int count_config_variables(const git_config *cfg, size_t *n)
{
    int err;
    git_config_iterator *iterator = NULL;

    *n = 0;

    err = git_config_iterator_new(&iterator, cfg);
    if (err < 0)
        return err;

    for (;;) {
        git_config_entry *entry;
        err = git_config_next(&entry, iterator);
        if (err < 0)
            break;
        (*n)++;
    }

    git_config_iterator_free(iterator);

    if (GIT_ITEROVER != err)
        return err;
    return 0;
}

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
    SEXP list;
    int err, i;
    size_t protected = 0;
    size_t n;
    git_config *cfg = NULL;
    git_config_iterator *iterator = NULL;
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

    n = length(variables);
    if (n) {
        names = getAttrib(variables, R_NamesSymbol);
        for (i = 0; i < length(variables); i++) {
            const char *key = CHAR(STRING_ELT(names, i));
            const char *value = NULL;

            if (!isNull(VECTOR_ELT(variables, i)))
                value = CHAR(STRING_ELT(VECTOR_ELT(variables, i), 0));

            if (value)
                err = git_config_set_string(cfg, key, value);
            else
                err = git_config_delete_entry(cfg, key);
            if (err < 0)
                goto cleanup;
        }
    }

    err = count_config_variables(cfg, &n);
    if (err < 0)
        goto cleanup;

    PROTECT(list = allocVector(VECSXP, n));
    protected++;
    PROTECT(names = allocVector(STRSXP, n));
    protected++;

    err = git_config_iterator_new(&iterator, cfg);
    if (err < 0)
        goto cleanup;

    for (i = 0; i < n; i++) {
        git_config_entry *entry = NULL;

        err = git_config_next(&entry, iterator);
        if (err < 0) {
            if (GIT_ITEROVER == err) {
                err = 0;
                break;
            }
            goto cleanup;
        }

        SET_STRING_ELT(names, i, mkChar(entry->name));
        SET_VECTOR_ELT(list, i, ScalarString(mkChar(entry->value)));
    }

    setAttrib(list, R_NamesSymbol, names);

cleanup:
    if (iterator)
        git_config_iterator_free(iterator);

    if (config)
        git_config_free(cfg);

    if (repository)
        git_repository_free(repository);

    if (protected)
        unprotect(protected);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return list;
}
