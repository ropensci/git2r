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

#include "git2.h"

#include "git2r_arg.h"
#include "git2r_clone.h"
#include "git2r_cred.h"
#include "git2r_error.h"

/**
 * Data structure to hold information when performing clone.
 */
typedef struct {
    int received_progress;
    int received_done;
    SEXP credentials;
} git2r_clone_data;

/**
 * Show progress of clone
 *
 * @param progress The clone progress data
 * @param payload A pointer to a git2r_clone_data data
 * structure
 * @return 0
 */
static int git2r_clone_progress(
    const git_transfer_progress *progress,
    void *payload)
{
    int kbytes = progress->received_bytes / 1024;
    git2r_clone_data *pd = (git2r_clone_data*)payload;

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
 * Callback if the remote host requires authentication in order to
 * connect to it
 *
 * @param cred The newly created credential object.
 * @param url The resource for which we are demanding a credential.
 * @param user_from_url The username that was embedded in a "user@host"
 * remote url, or NULL if not included.
 * @param allowed_types A bitmask stating which cred types are OK to return.
 * @param payload The payload provided when specifying this callback.
 * @return 0 on success, else -1.
 */
int git2r_clone_cred_acquire(
    git_cred **cred,
    const char *url,
    const char *username_from_url,
    unsigned int allowed_types,
    void *payload)
{
    return git2r_cred_acquire_cb(
        cred,
        url,
        username_from_url,
        allowed_types,
        ((git2r_clone_data*)payload)->credentials);
}

/**
 * Clone a remote repository
 *
 * @param url the remote repository to clone
 * @param local_path local directory to clone to
 * @param bare Create a bare repository.
 * @param branch The name of the branch to checkout. Default is NULL
 *        which means to use the remote's default branch.
 * @param credentials The credentials for remote repository access.
 * @param progress show progress
 * @return R_NilValue
 */
SEXP git2r_clone(
    SEXP url,
    SEXP local_path,
    SEXP bare,
    SEXP branch,
    SEXP credentials,
    SEXP progress)
{
    int err;
    git_repository *repository = NULL;
    git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
    git2r_clone_data payload = {0, 0, R_NilValue};

    if (git2r_arg_check_string(url))
        git2r_error(git2r_err_string_arg, __func__, "url");
    if (git2r_arg_check_string(local_path))
        git2r_error(git2r_err_string_arg, __func__, "local_path");
    if (git2r_arg_check_logical(bare))
        git2r_error(git2r_err_logical_arg, __func__, "bare");
    if (branch != R_NilValue && git2r_arg_check_string(branch))
        git2r_error(git2r_err_string_arg, __func__, "branch");
    if (git2r_arg_check_credentials(credentials))
        git2r_error(git2r_err_credentials_arg, __func__, "credentials");
    if (git2r_arg_check_logical(progress))
        git2r_error(git2r_err_logical_arg, __func__, "progress");

    checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
    clone_opts.checkout_opts = checkout_opts;
    payload.credentials = credentials;
    clone_opts.fetch_opts.callbacks.payload = &payload;
    clone_opts.fetch_opts.callbacks.credentials = &git2r_clone_cred_acquire;

    if (LOGICAL(bare)[0])
        clone_opts.bare = 1;

    if (branch != R_NilValue)
        clone_opts.checkout_branch = CHAR(STRING_ELT(branch, 0));

    if (LOGICAL(progress)[0]) {
        clone_opts.fetch_opts.callbacks.transfer_progress = &git2r_clone_progress;
        Rprintf("cloning into '%s'...\n", CHAR(STRING_ELT(local_path, 0)));
    }

    err = git_clone(&repository,
                    CHAR(STRING_ELT(url, 0)),
                    CHAR(STRING_ELT(local_path, 0)),
                    &clone_opts);

    if (repository)
        git_repository_free(repository);

    if (GIT_OK != err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return R_NilValue;
}
