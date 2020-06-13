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
#include "git2r_clone.h"
#include "git2r_cred.h"
#include "git2r_deprecated.h"
#include "git2r_error.h"
#include "git2r_transfer.h"

/**
 * Show progress of clone
 *
 * @param progress The clone progress data
 * @param payload A pointer to a git2r_clone_data data
 * structure
 * @return 0
 */
static int
git2r_clone_progress(
    const GIT2R_INDEXER_PROGRESS *progress,
    void *payload)
{
    int kbytes = progress->received_bytes / 1024;
    git2r_transfer_data *pd = (git2r_transfer_data*)payload;

    if (progress->received_objects < progress->total_objects) {
        int received_percent =
            (100 * progress->received_objects) /
            progress->total_objects;

        if (received_percent > pd->received_progress) {
            Rprintf("Receiving objects: % 3i%% (%i/%i), %4d kb\n",
                    received_percent,
                    progress->received_objects,
                    progress->total_objects,
                    kbytes);
            pd->received_progress += 10;
        }
    } else if (!pd->received_done) {
        Rprintf("Receiving objects: 100%% (%i/%i), %4d kb, done.\n",
                progress->received_objects,
                progress->total_objects,
                kbytes);
        pd->received_done = 1;
    }

    return 0;
}

/**
 * Clone a remote repository
 *
 * @param url the remote repository to clone
 * @param local_path local directory to clone to
 * @param bare Create a bare repository.
 * @param branch The name of the branch to checkout. Default is NULL
 *        which means to use the remote's default branch.
 * @param checkout Checkout HEAD after the clone is complete.
 * @param credentials The credentials for remote repository access.
 * @param progress show progress
 * @return R_NilValue
 */
SEXP attribute_hidden
git2r_clone(
    SEXP url,
    SEXP local_path,
    SEXP bare,
    SEXP branch,
    SEXP checkout,
    SEXP credentials,
    SEXP progress)
{
    int error;
    git_repository *repository = NULL;
    git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
    git2r_transfer_data payload = GIT2R_TRANSFER_DATA_INIT;

    if (git2r_arg_check_string(url))
        git2r_error(__func__, NULL, "'url'", git2r_err_string_arg);
    if (git2r_arg_check_string(local_path))
        git2r_error(__func__, NULL, "'local_path'", git2r_err_string_arg);
    if (git2r_arg_check_logical(bare))
        git2r_error(__func__, NULL, "'bare'", git2r_err_logical_arg);
    if ((!Rf_isNull(branch)) && git2r_arg_check_string(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_string_arg);
    if (git2r_arg_check_logical(checkout))
        git2r_error(__func__, NULL, "'checkout'", git2r_err_logical_arg);
    if (git2r_arg_check_credentials(credentials))
        git2r_error(__func__, NULL, "'credentials'", git2r_err_credentials_arg);
    if (git2r_arg_check_logical(progress))
        git2r_error(__func__, NULL, "'progress'", git2r_err_logical_arg);

    if (LOGICAL(checkout)[0])
      checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
    else
      checkout_opts.checkout_strategy = GIT_CHECKOUT_NONE;

    clone_opts.checkout_opts = checkout_opts;
    payload.credentials = credentials;
    clone_opts.fetch_opts.callbacks.payload = &payload;
    clone_opts.fetch_opts.callbacks.credentials = &git2r_cred_acquire_cb;

    if (LOGICAL(bare)[0])
        clone_opts.bare = 1;

    if (!Rf_isNull(branch))
        clone_opts.checkout_branch = CHAR(STRING_ELT(branch, 0));

    if (LOGICAL(progress)[0]) {
        clone_opts.fetch_opts.callbacks.transfer_progress = &git2r_clone_progress;
        Rprintf("cloning into '%s'...\n", CHAR(STRING_ELT(local_path, 0)));
    }

    error = git_clone(&repository,
                    CHAR(STRING_ELT(url, 0)),
                    CHAR(STRING_ELT(local_path, 0)),
                    &clone_opts);

    git_repository_free(repository);

    if (error)
        git2r_error(
            __func__,
            GIT2R_ERROR_LAST(),
            git2r_err_unable_to_authenticate,
            NULL);

    return R_NilValue;
}
