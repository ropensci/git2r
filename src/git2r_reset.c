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

#include "git2r_arg.h"
#include "git2r_commit.h"
#include "git2r_error.h"
#include "git2r_repository.h"
#include "git2r_reset.h"
#include "git2r_signature.h"

/**
 * Reset current HEAD to the specified state
 *
 * @param commit The commit to which the HEAD should be moved to.
 * @param reset_type Kind of reset operation to perform. 'soft' means
 * the Head will be moved to the commit. 'mixed' reset will trigger a
 * 'soft' reset, plus the index will be replaced with the content of
 * the commit tree. 'hard' reset will trigger a 'mixed' reset and the
 * working directory will be replaced with the content of the index.
 * @return R_NilValue
 */
SEXP git2r_reset(SEXP commit, SEXP reset_type)
{
    int err;
    SEXP repo;
    git_commit *target = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_commit(commit))
        git2r_error(git2r_err_commit_arg, __func__, "commit");
    if (git2r_arg_check_integer(reset_type))
        git2r_error(git2r_err_integer_arg, __func__, "reset_type");

    repo = GET_SLOT(commit, Rf_install("repo"));
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git2r_commit_lookup(&target, repository, commit);
    if (err)
        goto cleanup;

    err = git_reset(
        repository,
        (git_object*)target,
        INTEGER(reset_type)[0],
        NULL);

cleanup:
    if (target)
        git_commit_free(target);

    if (repository)
        git_repository_free(repository);

    if (err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return R_NilValue;
}
