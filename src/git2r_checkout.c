/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2015 The git2r contributors
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
#include "refs.h"

#include "git2r_arg.h"
#include "git2r_checkout.h"
#include "git2r_error.h"
#include "git2r_repository.h"

/**
 * Checkout path
 *
 * @param repo S4 class git_repository
 * @param path The paths to checkout
 * @return R_NilValue
 */
SEXP git2r_checkout_path(SEXP repo, SEXP path)
{
    int err = 0;
    size_t i, len;
    git_repository *repository = NULL;
    git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;

    if (git2r_arg_check_string_vec(path))
        git2r_error(__func__, NULL, "'path'", git2r_err_string_vec_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    /* Count number of non NA values */
    len = length(path);
    for (i = 0; i < len; i++)
        if (NA_STRING != STRING_ELT(path, i))
            opts.paths.count++;

    /* We are done if no non-NA values  */
    if (!opts.paths.count)
        goto cleanup;

    /* Allocate the strings in pathspec */
    opts.paths.strings = malloc(opts.paths.count * sizeof(char*));
    if (!opts.paths.strings) {
        giterr_set_str(GITERR_NONE, git2r_err_alloc_memory_buffer);
        err = GIT_ERROR;
        goto cleanup;
    }

    /* Populate the strings in opts.paths */
    for (i = 0; i < opts.paths.count; i++)
        if (NA_STRING != STRING_ELT(path, i))
            opts.paths.strings[i] = (char *)CHAR(STRING_ELT(path, i));

    opts.checkout_strategy = GIT_CHECKOUT_FORCE;
    err = git_checkout_head(repository, &opts);

cleanup:
    if (opts.paths.strings)
        free(opts.paths.strings);

    if (repository)
        git_repository_free(repository);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return R_NilValue;
}

/**
 * Checkout tree
 *
 * @param repo S4 class git_repository
 * @param revision The revision string, see
 * http://git-scm.com/docs/git-rev-parse.html#_specifying_revisions
 * @param force Using checkout strategy GIT_CHECKOUT_SAFE (force =
 *        FALSE) or GIT_CHECKOUT_FORCE (force = TRUE).
 * @return R_NilValue
 */
SEXP git2r_checkout_tree(SEXP repo, SEXP revision, SEXP force)
{
    int err;
    git_repository *repository = NULL;
    git_object *treeish = NULL;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

    if (git2r_arg_check_string(revision))
        git2r_error(__func__, NULL, "'revision'", git2r_err_string_arg);
    if (git2r_arg_check_logical(force))
        git2r_error(__func__, NULL, "'force'", git2r_err_logical_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    err = git_revparse_single(&treeish, repository, CHAR(STRING_ELT(revision, 0)));
    if (err)
        goto cleanup;

    switch (git_object_type(treeish)) {
    case GIT_OBJ_COMMIT:
    case GIT_OBJ_TAG:
    case GIT_OBJ_TREE:
        err = GIT_OK;
        break;
    default:
        giterr_set_str(GITERR_NONE, git2r_err_checkout_tree);
        err = GIT_ERROR;
        break;
    }
    if (err)
        goto cleanup;

    if (LOGICAL(force)[0])
        checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE;
    else
        checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;

    err = git_checkout_tree(repository, treeish, &checkout_opts);

cleanup:
    if (treeish)
        git_object_free(treeish);

    if (repository)
        git_repository_free(repository);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return R_NilValue;
}
