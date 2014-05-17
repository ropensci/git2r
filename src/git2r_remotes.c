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

#include "git2r_remotes.h"
#include "git2r_repository.h"
#include "git2r_error.h"

#include "git2.h"

SEXP remote_add(SEXP repo, SEXP name, SEXP url)
{
    int err = 0;
    git_repository *repository = NULL;
    git_remote *remote = NULL;

    if (!IS_CHARACTER(name) || LENGTH(name) != 1)
	error("Invalid remote name");

    if (!IS_CHARACTER(url) || LENGTH(url) != 1)
	error("Invalid remote url");

    if (!git_remote_is_valid_name(CHAR(STRING_ELT(name, 0))))
	error("Invalid remote name");

    repository = git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

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
	error("Error: %s\n", giterr_last()->message);

    return R_NilValue;
}

SEXP remote_rename(SEXP repo, SEXP oldname, SEXP newname)
{
    int err = 0;
    git_repository *repository = NULL;
    git_remote *remote = NULL;

    if (!IS_CHARACTER(oldname) || LENGTH(oldname) != 1)
	error("Invalid remote name");

    if (!IS_CHARACTER(newname) || LENGTH(newname) != 1)
	error("Invalid remote name");

    if (!git_remote_is_valid_name(CHAR(STRING_ELT(newname, 0))))
	error("Invalid remote name");

    repository = git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    err = git_remote_load(&remote,
			  repository,
			  CHAR(STRING_ELT(oldname, 0)));

    if (err != 0)
	goto cleanup;

    err = git_remote_rename(remote,
			    CHAR(STRING_ELT(newname, 0)),
			    /* callback= */ NULL,
			    /* payload= */ NULL);

cleanup:

    if (remote)
	git_remote_free(remote);

    if (repository)
	git_repository_free(repository);

    if (err != 0)
	error("Error: %s\n", giterr_last()->message);

    return R_NilValue;
}

SEXP remote_remove(SEXP repo, SEXP name)
{
    int err = 0;
    git_repository *repository = NULL;
    git_remote *remote = NULL;

    if (!IS_CHARACTER(name) || LENGTH(name) != 1)
	error("Invalid remote name");

    repository = git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    err = git_remote_load(&remote,
			  repository,
			  CHAR(STRING_ELT(name, 0)));

    if (err != 0)
	goto cleanup;

    err = git_remote_delete(remote);

    if (err != 0)
	goto cleanup;

    /* Was freed by git_remote_delete */
    remote = NULL;

cleanup:

    if (remote)
	git_remote_free(remote);

    if (repository)
	git_repository_free(repository);

    if (err != 0)
	error("Error: %s\n", giterr_last()->message);

    return R_NilValue;
}
