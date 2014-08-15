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

#include "git2.h"

#include "git2r_arg.h"
#include "git2r_error.h"
#include "git2r_remote.h"
#include "git2r_repository.h"

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
    int err = 0;
    git_repository *repository = NULL;
    git_remote *remote = NULL;

    if (0 != git2r_arg_check_string(name))
        git2r_error(git2r_err_string_arg, __func__, "name");
    if (0 != git2r_arg_check_string(url))
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

cleanup:
    if (remote)
	git_remote_free(remote);

    if (repository)
	git_repository_free(repository);

    if (err != 0)
	git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return R_NilValue;
}

/**
 * Fetch new data and update tips
 *
 * @param repo S4 class git_repository
 * @param name The name of the remote to fetch from
 * @return R_NilValue
 */
SEXP git2r_remote_fetch(SEXP repo, SEXP name)
{
    int err;
    git_remote *remote = NULL;
    git_repository *repository = NULL;

    if (0 != git2r_arg_check_string(name))
        git2r_error(git2r_err_string_arg, __func__, "name");

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git_remote_load(&remote, repository, CHAR(STRING_ELT(name, 0)));
    if (err < 0)
        goto cleanup;

    err = git_remote_fetch(remote, NULL, NULL);
    if (err < 0)
        goto cleanup;

cleanup:
    if (remote)
        git_remote_disconnect(remote);

    if (remote)
        git_remote_free(remote);

    if (repository)
        git_repository_free(repository);

    if (err < 0)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return R_NilValue;
}

/**
 * Get the configured remotes for a repo
 *
 * @param repo S4 class git_repository
 * @return Character vector with name of the remotes
 */
SEXP git2r_remote_list(SEXP repo)
{
    int i, err;
    git_strarray rem_list;
    SEXP list;
    git_repository *repository;

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git_remote_list(&rem_list, repository);
    if (err < 0)
        goto cleanup;

    PROTECT(list = allocVector(STRSXP, rem_list.count));
    for (i = 0; i < rem_list.count; i++)
        SET_STRING_ELT(list, i, mkChar(rem_list.strings[i]));
    UNPROTECT(1);

cleanup:
    git_strarray_free(&rem_list);

    if (repository)
        git_repository_free(repository);

    if (err < 0)
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
    int err = 0;
    git_repository *repository = NULL;
    git_remote *remote = NULL;

    if (0 != git2r_arg_check_string(name))
        git2r_error(git2r_err_string_arg, __func__, "name");

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git_remote_load(&remote,
			  repository,
			  CHAR(STRING_ELT(name, 0)));

    if (err != 0)
	goto cleanup;

    err = git_remote_delete(remote);

cleanup:
    if (remote)
	git_remote_free(remote);

    if (repository)
	git_repository_free(repository);

    if (err != 0)
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
    int err = 0;
    git_strarray problems;
    git_repository *repository = NULL;
    git_remote *remote = NULL;

    if (0 != git2r_arg_check_string(oldname))
        git2r_error(git2r_err_string_arg, __func__, "oldname");
    if (0 != git2r_arg_check_string(newname))
        git2r_error(git2r_err_string_arg, __func__, "newname");

    if (!git_remote_is_valid_name(CHAR(STRING_ELT(newname, 0))))
	git2r_error("Error in '%s': Invalid new remote name", __func__, NULL);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git_remote_load(&remote,
			  repository,
			  CHAR(STRING_ELT(oldname, 0)));

    if (err != 0)
	goto cleanup;

    err = git_remote_rename(&problems,
                            remote,
			    CHAR(STRING_ELT(newname, 0)));

    if (err != 0)
	goto cleanup;

    git_strarray_free(&problems);

cleanup:
    if (remote)
	git_remote_free(remote);

    if (repository)
	git_repository_free(repository);

    if (err != 0)
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
    int err;
    SEXP url;
    size_t len;
    size_t i = 0;
    git_remote *tmp_remote;
    git_repository *repository = NULL;

    if (0 != git2r_arg_check_string_vec(remote))
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
            err = git_remote_load(&tmp_remote,
                                  repository,
                                  CHAR(STRING_ELT(remote, i)));
            if (err < 0)
                goto cleanup;

            SET_STRING_ELT(url, i, mkChar(git_remote_url(tmp_remote)));
            git_remote_free(tmp_remote);
        }
    }

cleanup:
    if (repository)
        git_repository_free(repository);

    UNPROTECT(1);

    if (err < 0)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return url;
}
