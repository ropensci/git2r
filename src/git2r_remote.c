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

#include "git2r_arg.h"
#include "git2r_cred.h"
#include "git2r_error.h"
#include "git2r_remote.h"
#include "git2r_repository.h"
#include "git2r_signature.h"
#include "git2r_transfer.h"

/**
 * Add a remote with the default fetch refspec to the repository's
 * configuration.
 *
 * @param repo S4 class git_repository
 * @param name The name of the remote
 * @param url The url of the remote
 * @return R_NilValue
 */
SEXP git2r_remote_add(SEXP repo, SEXP name, SEXP url)
{
    int err;
    git_repository *repository = NULL;
    git_remote *remote = NULL;

    if (GIT_OK != git2r_arg_check_string(name))
        git2r_error(git2r_err_string_arg, __func__, "name");
    if (GIT_OK != git2r_arg_check_string(url))
        git2r_error(git2r_err_string_arg, __func__, "url");

    if (!git_remote_is_valid_name(CHAR(STRING_ELT(name, 0))))
	git2r_error("Error in '%s': Invalid remote name", __func__, NULL);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git_remote_create(&remote,
			    repository,
			    CHAR(STRING_ELT(name, 0)),
			    CHAR(STRING_ELT(url, 0)));

    if (remote)
	git_remote_free(remote);

    if (repository)
	git_repository_free(repository);

    if (GIT_OK != err)
	git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return R_NilValue;
}

/**
 * Fetch new data and update tips
 *
 * @param repo S4 class git_repository
 * @param name The name of the remote to fetch from
 * @param credentials The credentials for remote repository access.
 * @param msg The one line long message to be appended to the reflog
 * @param who The identity that will used to populate the reflog entry
 * @return R_NilValue
 */
SEXP git2r_remote_fetch(
    SEXP repo,
    SEXP name,
    SEXP credentials,
    SEXP msg,
    SEXP who)
{
    int err;
    SEXP result = R_NilValue;
    const git_transfer_progress *stats;
    git_remote *remote = NULL;
    git_signature *signature = NULL;
    git_repository *repository = NULL;
    git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;

    if (GIT_OK != git2r_arg_check_string(name))
        git2r_error(git2r_err_string_arg, __func__, "name");
    if (GIT_OK != git2r_arg_check_credentials(credentials))
        git2r_error(git2r_err_credentials_arg, __func__, "credentials");
    if (GIT_OK != git2r_arg_check_string(msg))
        git2r_error(git2r_err_string_arg, __func__, "msg");
    if (GIT_OK != git2r_arg_check_signature(who))
        git2r_error(git2r_err_signature_arg, __func__, "who");

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git2r_signature_from_arg(&signature, who);
    if (GIT_OK != err)
        goto cleanup;

    err = git_remote_lookup(&remote, repository, CHAR(STRING_ELT(name, 0)));
    if (GIT_OK != err)
        goto cleanup;

    callbacks.credentials = &git2r_cred_acquire_cb;
    callbacks.payload = credentials;
    err = git_remote_set_callbacks(remote, &callbacks);
    if (GIT_OK != err)
        goto cleanup;

    err = git_remote_fetch(remote, NULL, signature, CHAR(STRING_ELT(msg, 0)));
    if (GIT_OK != err)
        goto cleanup;

    stats = git_remote_stats(remote);
    PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_transfer_progress")));
    git2r_transfer_progress_init(stats, result);

cleanup:
    if (signature)
        git_signature_free(signature);

    if (remote) {
        if (git_remote_connected(remote))
            git_remote_disconnect(remote);
        git_remote_free(remote);
    }

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (GIT_OK != err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return result;
}

/**
 * Get the configured remotes for a repo
 *
 * @param repo S4 class git_repository
 * @return Character vector with name of the remotes
 */
SEXP git2r_remote_list(SEXP repo)
{
    int err;
    size_t i;
    git_strarray rem_list;
    SEXP list = R_NilValue;
    git_repository *repository;

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git_remote_list(&rem_list, repository);
    if (GIT_OK != err)
        goto cleanup;

    PROTECT(list = allocVector(STRSXP, rem_list.count));
    for (i = 0; i < rem_list.count; i++)
        SET_STRING_ELT(list, i, mkChar(rem_list.strings[i]));

cleanup:
    git_strarray_free(&rem_list);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != list)
        UNPROTECT(1);

    if (GIT_OK != err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return list;
}

/**
 * Remove an existing remote
 *
 * All remote-tracking branches and configuration settings for the
 * remote will be removed.
 * @param repo S4 class git_repository
 * @param name The name of the remote to remove
 * @return R_NilValue
 */
SEXP git2r_remote_remove(SEXP repo, SEXP name)
{
    int err;
    git_repository *repository = NULL;

    if (GIT_OK != git2r_arg_check_string(name))
        git2r_error(git2r_err_string_arg, __func__, "name");

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git_remote_delete(repository, CHAR(STRING_ELT(name, 0)));

    if (repository)
	git_repository_free(repository);

    if (GIT_OK != err)
	git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return R_NilValue;
}

/**
 * Give the remote a new name
 *
 * @param repo S4 class git_repository
 * @param oldname The old name of the remote
 * @param newname The new name of the remote
 * @return R_NilValue
 */
SEXP git2r_remote_rename(SEXP repo, SEXP oldname, SEXP newname)
{
    int err;
    git_strarray problems = {0};
    git_repository *repository = NULL;

    if (GIT_OK != git2r_arg_check_string(oldname))
        git2r_error(git2r_err_string_arg, __func__, "oldname");
    if (GIT_OK != git2r_arg_check_string(newname))
        git2r_error(git2r_err_string_arg, __func__, "newname");

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git_remote_rename(
        &problems,
        repository,
        CHAR(STRING_ELT(oldname, 0)),
        CHAR(STRING_ELT(newname, 0)));
    if (GIT_OK != err)
	goto cleanup;

    git_strarray_free(&problems);

cleanup:
    if (repository)
	git_repository_free(repository);

    if (GIT_OK != err)
	git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return R_NilValue;
}

/**
 * Get the remote's url
 *
 * @param repo S4 class git_repository
 * @param remote Character vector with name of remote. NA values are
 * ok and give NA values as result at corresponding index in url
 * vector
 * @return Character vector with url for each remote
 */
SEXP git2r_remote_url(SEXP repo, SEXP remote)
{
    int err = GIT_OK;
    SEXP url;
    size_t len;
    size_t i = 0;
    git_remote *tmp_remote;
    git_repository *repository = NULL;

    if (GIT_OK != git2r_arg_check_string_vec(remote))
        git2r_error(git2r_err_string_vec_arg, __func__, "remote");

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    len = LENGTH(remote);
    PROTECT(url = allocVector(STRSXP, len));

    for (; i < len; i++) {
        if (NA_STRING == STRING_ELT(remote, i)) {
            SET_STRING_ELT(url, i, NA_STRING);
        } else {
            err = git_remote_lookup(
                &tmp_remote,
                repository,
                CHAR(STRING_ELT(remote, i)));
            if (GIT_OK != err)
                goto cleanup;

            SET_STRING_ELT(url, i, mkChar(git_remote_url(tmp_remote)));
            git_remote_free(tmp_remote);
        }
    }

cleanup:
    if (repository)
        git_repository_free(repository);

    UNPROTECT(1);

    if (GIT_OK != err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return url;
}
