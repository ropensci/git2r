/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2020 The git2r contributors
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

#include <R_ext/Visibility.h>
#include <git2.h>

#include "git2r_arg.h"
#include "git2r_checkout.h"
#include "git2r_deprecated.h"
#include "git2r_error.h"
#include "git2r_repository.h"

/**
 * Checkout path
 *
 * @param repo S3 class git_repository
 * @param path The paths to checkout
 * @return R_NilValue
 */
SEXP attribute_hidden
git2r_checkout_path(
    SEXP repo,
    SEXP path)
{
    int error = 0;
    size_t i, len;
    git_repository *repository = NULL;
    git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;

    if (git2r_arg_check_string_vec(path))
        git2r_error(__func__, NULL, "'path'", git2r_err_string_vec_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    /* Count number of non NA values */
    len = Rf_length(path);
    for (i = 0; i < len; i++)
        if (NA_STRING != STRING_ELT(path, i))
            opts.paths.count++;

    /* We are done if no non-NA values  */
    if (!opts.paths.count)
        goto cleanup;

    /* Allocate the strings in pathspec */
    opts.paths.strings = malloc(opts.paths.count * sizeof(char*));
    if (!opts.paths.strings) {
        GIT2R_ERROR_SET_STR(GIT2R_ERROR_NONE, git2r_err_alloc_memory_buffer);
        error = GIT_ERROR;
        goto cleanup;
    }

    /* Populate the strings in opts.paths */
    for (i = 0; i < opts.paths.count; i++)
        if (NA_STRING != STRING_ELT(path, i))
            opts.paths.strings[i] = (char *)CHAR(STRING_ELT(path, i));

    opts.checkout_strategy = GIT_CHECKOUT_FORCE;
    error = git_checkout_head(repository, &opts);

cleanup:
    free(opts.paths.strings);
    git_repository_free(repository);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return R_NilValue;
}

/**
 * Checkout tree
 *
 * @param repo S3 class git_repository
 * @param revision The revision string, see
 * http://git-scm.com/docs/git-rev-parse.html#_specifying_revisions
 * @param force Using checkout strategy GIT_CHECKOUT_SAFE (force =
 *        FALSE) or GIT_CHECKOUT_FORCE (force = TRUE).
 * @return R_NilValue
 */
SEXP attribute_hidden
git2r_checkout_tree(
    SEXP repo,
    SEXP revision,
    SEXP force)
{
    int error;
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

    error = git_revparse_single(&treeish, repository, CHAR(STRING_ELT(revision, 0)));
    if (error)
        goto cleanup;

    switch (git_object_type(treeish)) {
    case GIT2R_OBJECT_COMMIT:
    case GIT2R_OBJECT_TAG:
    case GIT2R_OBJECT_TREE:
        error = GIT_OK;
        break;
    default:
        GIT2R_ERROR_SET_STR(GIT2R_ERROR_NONE, git2r_err_checkout_tree);
        error = GIT_ERROR;
        break;
    }
    if (error)
        goto cleanup;

    if (LOGICAL(force)[0])
        checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE;
    else
        checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;

    error = git_checkout_tree(repository, treeish, &checkout_opts);

cleanup:
    git_object_free(treeish);
    git_repository_free(repository);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return R_NilValue;
}
