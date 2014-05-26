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

#define GIT2R_N_CONFIG_LEVELS 6

/**
 * Count number of config variables by level
 *
 * @param cfg where to count the variables
 * @param n_level array to store the number of variables
 * @param 0 on succes, or error code
 */
static int git2r_config_count_variables(
    const git_config *cfg,
    size_t *n_level,
    const char **err_msg)
{
    int err;
    git_config_iterator *iterator = NULL;

    err = git_config_iterator_new(&iterator, cfg);
    if (err < 0)
        return err;

    for (;;) {
        git_config_entry *entry;
        err = git_config_next(&entry, iterator);
        if (err < 0)
            break;

        switch (entry->level) {
        case GIT_CONFIG_LEVEL_SYSTEM:
            n_level[0]++;
            break;
        case GIT_CONFIG_LEVEL_XDG:
            n_level[1]++;
            break;
        case GIT_CONFIG_LEVEL_GLOBAL:
            n_level[2]++;
            break;
        case GIT_CONFIG_LEVEL_LOCAL:
            n_level[3]++;
            break;
        case GIT_CONFIG_LEVEL_APP:
            n_level[4]++;
            break;
        case GIT_CONFIG_HIGHEST_LEVEL:
            n_level[5]++;
            break;
        default:
            *err_msg = git2r_err_unexpected_config_level;
            goto cleanup;
        }
    }

cleanup:
    if (iterator)
        git_config_iterator_free(iterator);

    if (*err_msg)
        return -1;

    if (GIT_ITEROVER != err)
        return err;

    return 0;
}

/**
 * Intialize a list for a config level. The list is only created if
 * there are any entries at that level.
 *
 * @param level the index of the level
 * @param n_level vector with number of entries per level
 * @param name name of the level to initialize
 * @return index of the config level list in the owning list
 */
static size_t git2r_config_list_init(
    SEXP list,
    size_t level,
    size_t *n_level,
    size_t *i_list,
    size_t i,
    const char *name)
{
    if (n_level[level]) {
        SEXP tmp, names;

        PROTECT(tmp = allocVector(VECSXP, n_level[level]));
        setAttrib(tmp, R_NamesSymbol, allocVector(STRSXP, n_level[level]));
        i_list[level] = i++;
        names = getAttrib(list, R_NamesSymbol);
        SET_STRING_ELT(names, i_list[level] , mkChar(name));
        SET_VECTOR_ELT(list, i_list[level], tmp);
        UNPROTECT(1);
    }

    return i;
}

/**
 * Add entry to result list.
 *
 * @param list the result list
 * @param level the level of the entry
 * @param i_level the index where to add the entry within the level
 * @param n_level
 * @param i_list
 * @param entry the config entry to add
 * @return void
 */
static void git2r_config_list_add_entry(
    SEXP list,
    size_t level,
    size_t *i_level,
    size_t *i_list,
    git_config_entry *entry)
{
    if (i_list[level] < LENGTH(list)) {
        SEXP sub_list = VECTOR_ELT(list, i_list[level]);

        if (i_level[level] < LENGTH(sub_list)) {
            SEXP names = getAttrib(sub_list, R_NamesSymbol);
            SET_STRING_ELT(names, i_level[level], mkChar(entry->name));
            SET_VECTOR_ELT(sub_list,
                           i_level[level],
                           ScalarString(mkChar(entry->value)));
            i_level[level]++;
            return;
        }
    }

    error("Unexpected");
}

/**
 * List config variables
 *
 * @param cfg
 * @param list
 * @param n_level
 * @param err_msg
 * @return
 */
static int git2r_config_list_variables(
    git_config *cfg,
    SEXP list,
    size_t *n_level,
    const char **err_msg)
{
    int err;
    size_t i_level[GIT2R_N_CONFIG_LEVELS] = {0}; /* Current index at level */
    size_t i_list[GIT2R_N_CONFIG_LEVELS] = {0};  /* Index of level in target list */
    git_config_iterator *iterator = NULL;
    size_t i = 0;

    err = git_config_iterator_new(&iterator, cfg);
    if (err < 0)
        goto cleanup;

    i = git2r_config_list_init(list, 0, n_level, i_list, i, "system");
    i = git2r_config_list_init(list, 1, n_level, i_list, i, "xdg");
    i = git2r_config_list_init(list, 2, n_level, i_list, i, "global");
    i = git2r_config_list_init(list, 3, n_level, i_list, i, "local");
    i = git2r_config_list_init(list, 4, n_level, i_list, i, "app");
    i = git2r_config_list_init(list, 5, n_level, i_list, i, "highest");

    for (;;) {
        git_config_entry *entry;
        err = git_config_next(&entry, iterator);
        if (err < 0) {
            if (GIT_ITEROVER == err) {
                err = 0;
                break;
            }
            goto cleanup;
        }

        switch (entry->level) {
        case GIT_CONFIG_LEVEL_SYSTEM:
            git2r_config_list_add_entry(list, 0, i_level, i_list, entry);
            break;
        case GIT_CONFIG_LEVEL_XDG:
            git2r_config_list_add_entry(list, 1, i_level, i_list, entry);
            break;
        case GIT_CONFIG_LEVEL_GLOBAL:
            git2r_config_list_add_entry(list, 2, i_level, i_list, entry);
            break;
        case GIT_CONFIG_LEVEL_LOCAL:
            git2r_config_list_add_entry(list, 3, i_level, i_list, entry);
            break;
        case GIT_CONFIG_LEVEL_APP:
            git2r_config_list_add_entry(list, 4, i_level, i_list, entry);
            break;
        case GIT_CONFIG_HIGHEST_LEVEL:
            git2r_config_list_add_entry(list, 5, i_level, i_list, entry);
            break;
        default:
            *err_msg = git2r_err_unexpected_config_level;
            goto cleanup;
        }
    }

cleanup:
    if (iterator)
        git_config_iterator_free(iterator);

    if (*err_msg)
        return -1;

    return err;
}

/**
 * Get config variables
 *
 * @param repo S4 class git_repository
 * @return VECSXP list with variables by level
 */
SEXP git2r_config_get(SEXP repo)
{
    int err;
    SEXP list = R_NilValue;
    const char *err_msg = NULL;
    size_t i = 0, n = 0, n_level[GIT2R_N_CONFIG_LEVELS] = {0};
    git_config *cfg = NULL;
    git_repository *repository = NULL;

    repository = git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    err = git_repository_config(&cfg, repository);
    if (err < 0)
        goto cleanup;

    err = git2r_config_count_variables(cfg, n_level, &err_msg);
    if (err < 0)
        goto cleanup;

    /* Count levels with entries */
    for (; i < GIT2R_N_CONFIG_LEVELS; i++) {
        if (n_level[i])
            n++;
    }

    PROTECT(list = allocVector(VECSXP, n));
    setAttrib(list, R_NamesSymbol, allocVector(STRSXP, n));

    if (git2r_config_list_variables(cfg, list, n_level, &err_msg))
        goto cleanup;

cleanup:
    if (cfg)
        git_config_free(cfg);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != list)
        UNPROTECT(1);

    if (err < 0) {
        if (err_msg)
            error("Error: %s\n", err_msg);
        else
            error("Error: %s\n", giterr_last()->message);
    }

    return list;
}

/**
 * Set or delete config entries
 *
 * @param repo S4 class git_repository
 * @param variables list of variables. If variable is NULL, it's deleted.
 * @return R_NilValue
 */
SEXP git2r_config_set(SEXP repo, SEXP variables)
{
    int err;
    SEXP names;
    const char *err_msg = NULL;
    size_t i, n;
    git_config *cfg = NULL;
    git_repository *repository = NULL;

    if (R_NilValue == variables || !isNewList(variables))
        error("Invalid arguments to git2r_config_set");

    n = length(variables);
    if (n) {
        repository = git2r_repository_open(repo);
        if (!repository)
            error(git2r_err_invalid_repository);

        err = git_repository_config(&cfg, repository);
        if (err < 0)
            goto cleanup;

        names = getAttrib(variables, R_NamesSymbol);
        for (i = 0; i < n; i++) {
            int err;
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

cleanup:
    if (cfg)
        git_config_free(cfg);

    if (repository)
        git_repository_free(repository);

    if (err < 0) {
        if (err_msg)
            error("Error: %s\n", err_msg);
        else
            error("Error: %s\n", giterr_last()->message);
    }

    return R_NilValue;
}
