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
#include "git2r_error.h"
#include "git2r_remote.h"

/**
 * Fetch
 *
 * @param repo
 * @param name
 * @return R_NilValue
 */
SEXP git2r_remote_fetch(SEXP repo, SEXP name)
{
    int err;
    git_remote *remote = NULL;
    git_repository *repository = NULL;

    if (git2r_error_check_string_arg(name))
        error("Invalid arguments to fetch");

    repository = git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

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

    return R_NilValue;
}

/**
 * Get the configured remotes for a repo
 *
 * @param repo S4 class git_repository
 * @return
 */
SEXP git2r_remote_list(SEXP repo)
{
    int i, err;
    git_strarray rem_list;
    SEXP list;
    git_repository *repository;

    repository = git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

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
        error("Error: %s\n", giterr_last()->message);

    return list;
}

/**
 * Get the remote's url
 *
 * @param repo S4 class git_repository
 * @return
 */
SEXP git2r_remote_url(SEXP repo, SEXP remote)
{
    int err;
    SEXP url;
    size_t len = LENGTH(remote);
    size_t i = 0;
    git_repository *repository;

    repository = git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    PROTECT(url = allocVector(STRSXP, len));

    for (; i < len; i++) {
        git_remote *r;

        err = git_remote_load(&r, repository, CHAR(STRING_ELT(remote, i)));
        if (err < 0)
            goto cleanup;

        SET_STRING_ELT(url, i, mkChar(git_remote_url(r)));
        git_remote_free(r);
    }

cleanup:
    git_repository_free(repository);

    UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return url;
}
