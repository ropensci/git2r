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
#include "git2r_cred.h"
#include "git2r_deprecated.h"
#include "git2r_error.h"
#include "git2r_push.h"
#include "git2r_repository.h"
#include "git2r_signature.h"
#include "git2r_transfer.h"

/**
 * Check if any non NA refspec
 *
 * @param refspec The string vector of refspec to push
 * @return 1 if nothing to push else 0
 */
static int
git2r_nothing_to_push(
    SEXP refspec)
{
    size_t i, n;

    n = Rf_length(refspec);
    if (0 == n)
        return 1; /* Nothing to push */

    for (i = 0; i < n; i++) {
        if (NA_STRING != STRING_ELT(refspec, i))
            return 0;
    }

    /* Nothing to push */
    return 1;
}

/**
 * Push
 *
 * @param repo S3 class git_repository
 * @param name The remote to push to
 * @param refspec The string vector of refspec to push
 * @param credentials The credentials for remote repository access.
 * @return R_NilValue
 */
SEXP attribute_hidden
git2r_push(
    SEXP repo,
    SEXP name,
    SEXP refspec,
    SEXP credentials)
{
    int error;
    git_remote *remote = NULL;
    git_repository *repository = NULL;
    git_strarray c_refspecs = {0};
    git_push_options opts = GIT_PUSH_OPTIONS_INIT;
    git2r_transfer_data payload = GIT2R_TRANSFER_DATA_INIT;

    if (git2r_arg_check_string(name))
        git2r_error(__func__, NULL, "'name'", git2r_err_string_arg);
    if (git2r_arg_check_string_vec(refspec))
        git2r_error(__func__, NULL, "'refspec'", git2r_err_string_vec_arg);
    if (git2r_arg_check_credentials(credentials))
        git2r_error(__func__, NULL, "'credentials'", git2r_err_credentials_arg);

    if (git2r_nothing_to_push(refspec))
        return R_NilValue;

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git_remote_lookup(&remote, repository, CHAR(STRING_ELT(name, 0)));
    if (error)
        goto cleanup;

    payload.credentials = credentials;
    opts.callbacks.payload = &payload;
    opts.callbacks.credentials = &git2r_cred_acquire_cb;

    error = git2r_copy_string_vec(&c_refspecs, refspec);
    if (error)
        goto cleanup;

    error = git_remote_push(remote, &c_refspecs, &opts);

cleanup:
    free(c_refspecs.strings);

    if (remote) {
        if (git_remote_connected(remote))
            git_remote_disconnect(remote);
        git_remote_free(remote);
    }

    git_repository_free(repository);

    if (error)
        git2r_error(
            __func__,
            GIT2R_ERROR_LAST(),
            git2r_err_unable_to_authenticate, NULL);

    return R_NilValue;
}
