/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2016 The git2r contributors
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
#include "common.h"

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

    if (git2r_arg_check_string(name))
        git2r_error(__func__, NULL, "'name'", git2r_err_string_arg);
    if (git2r_arg_check_string(url))
        git2r_error(__func__, NULL, "'url'", git2r_err_string_arg);

    if (!git_remote_is_valid_name(CHAR(STRING_ELT(name, 0))))
	git2r_error(__func__, NULL, git2r_err_invalid_remote, NULL);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    err = git_remote_create(
        &remote,
        repository,
        CHAR(STRING_ELT(name, 0)),
        CHAR(STRING_ELT(url, 0)));

    if (remote)
	git_remote_free(remote);

    if (repository)
	git_repository_free(repository);

    if (err)
	git2r_error(__func__, giterr_last(), NULL, NULL);

    return R_NilValue;
}

/**
 * Each time a reference is updated locally, this function will be
 * called with information about it.
 *
 * Based on the libgit2 network/fetch.c example.
 *
 * @param refname The name of the remote
 * @param a The previous position of branch
 * @param b The new position of branch
 * @param payload Callback data.
 * @return 0
 */
static int git2r_update_tips_cb(
    const char *refname,
    const git_oid *a,
    const git_oid *b,
    void *payload)
{
    git2r_transfer_data *cb_data = (git2r_transfer_data*)payload;

    if (cb_data->verbose) {
        char b_str[GIT_OID_HEXSZ + 1];
        git_oid_fmt(b_str, b);
        b_str[GIT_OID_HEXSZ] = '\0';

        if (git_oid_iszero(a)) {
            Rprintf("[new]     %.20s %s\n", b_str, refname);
        } else {
            char a_str[GIT_OID_HEXSZ + 1];
            git_oid_fmt(a_str, a);
            a_str[GIT_OID_HEXSZ] = '\0';
            Rprintf("[updated] %.10s..%.10s %s\n", a_str, b_str, refname);
        }
    }

    return 0;
}

/**
 * Fetch new data and update tips
 *
 * @param repo S4 class git_repository
 * @param name The name of the remote to fetch from
 * @param credentials The credentials for remote repository access.
 * @param msg The one line long message to be appended to the reflog
 * @param verbose Print information each time a reference is updated locally.
 * @param refspecs The refspecs to use for this fetch. Pass R_NilValue
 *        to use the base refspecs.
 * @return R_NilValue
 */
SEXP git2r_remote_fetch(
    SEXP repo,
    SEXP name,
    SEXP credentials,
    SEXP msg,
    SEXP verbose,
    SEXP refspecs)
{
    int err;
    SEXP result = R_NilValue;
    const git_transfer_progress *stats;
    git_remote *remote = NULL;
    git_repository *repository = NULL;
    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
    git2r_transfer_data payload = GIT2R_TRANSFER_DATA_INIT;
    git_strarray refs = {0};

    if (git2r_arg_check_string(name))
        git2r_error(__func__, NULL, "'name'", git2r_err_string_arg);
    if (git2r_arg_check_credentials(credentials))
        git2r_error(__func__, NULL, "'credentials'", git2r_err_credentials_arg);
    if (git2r_arg_check_string(msg))
        git2r_error(__func__, NULL, "'msg'", git2r_err_string_arg);
    if (git2r_arg_check_logical(verbose))
        git2r_error(__func__, NULL, "'verbose'", git2r_err_logical_arg);
    if (refspecs != R_NilValue && git2r_arg_check_string_vec(refspecs))
        git2r_error(__func__, NULL, "'refspecs'", git2r_err_string_vec_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    err = git_remote_lookup(&remote, repository, CHAR(STRING_ELT(name, 0)));
    if (err)
        goto cleanup;

    if (refspecs != R_NilValue) {
        size_t i, len;

        /* Count number of non NA values */
        len = length(refspecs);
        for (i = 0; i < len; i++)
            if (NA_STRING != STRING_ELT(refspecs, i))
                refs.count++;

        if (refs.count) {
            /* Allocate the strings in refs */
            refs.strings = malloc(refs.count * sizeof(char*));
            if (!refs.strings) {
                giterr_set_str(GITERR_NONE, git2r_err_alloc_memory_buffer);
                err = GIT_ERROR;
                goto cleanup;
            }

            /* Populate the strings in refs */
            for (i = 0; i < refs.count; i++)
                if (NA_STRING != STRING_ELT(refspecs, i))
                    refs.strings[i] = (char *)CHAR(STRING_ELT(refspecs, i));
        }
    }

    if (LOGICAL(verbose)[0])
        payload.verbose = 1;
    payload.credentials = credentials;
    fetch_opts.callbacks.payload = &payload;
    fetch_opts.callbacks.credentials = &git2r_cred_acquire_cb;
    fetch_opts.callbacks.update_tips = &git2r_update_tips_cb;
    err = git_remote_fetch(remote, &refs, &fetch_opts, CHAR(STRING_ELT(msg, 0)));
    if (err)
        goto cleanup;

    stats = git_remote_stats(remote);
    PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_transfer_progress")));
    git2r_transfer_progress_init(stats, result);

cleanup:
    if (refs.strings)
        free(refs.strings);

    if (remote) {
        if (git_remote_connected(remote))
            git_remote_disconnect(remote);
        git_remote_free(remote);
    }

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err)
        git2r_error(
            __func__,
            giterr_last(),
            git2r_err_unable_to_authenticate,
            NULL);

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
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    err = git_remote_list(&rem_list, repository);
    if (err)
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

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

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

    if (git2r_arg_check_string(name))
        git2r_error(__func__, NULL, "'name'", git2r_err_string_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    err = git_remote_delete(repository, CHAR(STRING_ELT(name, 0)));

    if (repository)
	git_repository_free(repository);

    if (err)
	git2r_error(__func__, giterr_last(), NULL, NULL);

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

    if (git2r_arg_check_string(oldname))
        git2r_error(__func__, NULL, "'oldname'", git2r_err_string_arg);
    if (git2r_arg_check_string(newname))
        git2r_error(__func__, NULL, "'newname'", git2r_err_string_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    err = git_remote_rename(
        &problems,
        repository,
        CHAR(STRING_ELT(oldname, 0)),
        CHAR(STRING_ELT(newname, 0)));
    if (err)
	goto cleanup;

    git_strarray_free(&problems);

cleanup:
    if (repository)
	git_repository_free(repository);

    if (err)
	git2r_error(__func__, giterr_last(), NULL, NULL);

    return R_NilValue;
}

/**
 * Set the remote's url in the configuration
 *
 * This assumes the common case of a single-url remote and
 * will otherwise raise an error.
 * @param repo S4 class git_repository
 * @param name The name of the remote
 * @param url The url to set
 * @return R_NilValue
 */
SEXP git2r_remote_set_url(SEXP repo, SEXP name, SEXP url)
{
    int err;
    git_repository *repository = NULL;

    if (git2r_arg_check_string(name))
        git2r_error(__func__, NULL, "'name'", git2r_err_string_arg);
    if (git2r_arg_check_string(url))
        git2r_error(__func__, NULL, "'url'", git2r_err_string_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    err = git_remote_set_url(
        repository,
        CHAR(STRING_ELT(name, 0)),
        CHAR(STRING_ELT(url, 0)));

    if (repository)
	git_repository_free(repository);

    if (err)
	git2r_error(__func__, giterr_last(), NULL, NULL);

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

    if (git2r_arg_check_string_vec(remote))
        git2r_error(__func__, NULL, "'remote'", git2r_err_string_vec_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

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
            if (err)
                goto cleanup;

            SET_STRING_ELT(url, i, mkChar(git_remote_url(tmp_remote)));
            git_remote_free(tmp_remote);
        }
    }

cleanup:
    if (repository)
        git_repository_free(repository);

    UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return url;
}

/**
 * Get the remote's url
 *
 * Based on https://github.com/libgit2/libgit2/blob/babdc376c7/examples/network/ls-remote.c
 * @param repo S4 class git_repository
 * @param name Character vector with URL of remote.
 * @return Character vector for each reference with the associated commit IDs.
 */
SEXP git2r_remote_ls(SEXP name, SEXP repo, SEXP credentials)
{
    const char *name_ = NULL;
    SEXP result = R_NilValue;
    SEXP names = R_NilValue;
    git_remote *remote = NULL;
    int err;
    const git_remote_head **refs;
    size_t refs_len, i;
    git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
    git2r_transfer_data payload = GIT2R_TRANSFER_DATA_INIT;
    git_repository *repository = NULL;

    if (git2r_arg_check_string(name))
        git2r_error(__func__, NULL, "'name'", git2r_err_string_arg);
    if (git2r_arg_check_credentials(credentials))
        git2r_error(__func__, NULL, "'credentials'", git2r_err_credentials_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    name_ = CHAR(STRING_ELT(name, 0));
    err = git_remote_lookup(&remote, repository, name_);
    if (err) {
        err = git_remote_create_anonymous(&remote, repository, name_);
        if (err)
            goto cleanup;
    }

    payload.credentials = credentials;
    callbacks.payload = &payload;
    callbacks.credentials = &git2r_cred_acquire_cb;

    err = git_remote_connect(remote, GIT_DIRECTION_FETCH, &callbacks, NULL);
    if (err)
        goto cleanup;

    err = git_remote_ls(&refs, &refs_len, remote);
    if (err)
        goto cleanup;

    PROTECT(result = allocVector(STRSXP, refs_len));
    setAttrib(result, R_NamesSymbol, names = allocVector(STRSXP, refs_len));

    for (i = 0; i < refs_len; i++) {
        char oid[GIT_OID_HEXSZ + 1] = {0};
        git_oid_fmt(oid, &refs[i]->oid);
        SET_STRING_ELT(result, i, mkChar(oid));
        SET_STRING_ELT(names, i, mkChar(refs[i]->name));
    }

cleanup:
    if (repository)
        git_repository_free(repository);

    if (result != R_NilValue)
        UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return(result);
}
