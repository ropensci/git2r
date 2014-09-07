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
#include "git2r_commit.h"
#include "git2r_error.h"
#include "git2r_merge.h"
#include "git2r_repository.h"

/**
 * Find a merge base between two commits
 *
 * @param one One of the commits
 * @param two The other commit
 * @return The commit of a merge base between 'one' and 'two'
 *         or NULL if not found
 */
SEXP git2r_merge_base(SEXP one, SEXP two)
{
    int err;
    SEXP result = R_NilValue;
    SEXP repo;
    SEXP sha;
    git_oid oid;
    git_oid oid_one;
    git_oid oid_two;
    git_commit *commit = NULL;
    git_repository *repository = NULL;

    if (GIT_OK != git2r_arg_check_commit(one))
        git2r_error(git2r_err_commit_arg, __func__, "one");
    if (GIT_OK != git2r_arg_check_commit(two))
        git2r_error(git2r_err_commit_arg, __func__, "two");

    repo = GET_SLOT(one, Rf_install("repo"));
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    sha = GET_SLOT(one, Rf_install("sha"));
    err = git_oid_fromstr(&oid_one, CHAR(STRING_ELT(sha, 0)));
    if (GIT_OK != err)
        goto cleanup;

    sha = GET_SLOT(two, Rf_install("sha"));
    err = git_oid_fromstr(&oid_two, CHAR(STRING_ELT(sha, 0)));
    if (GIT_OK != err)
        goto cleanup;

    err = git_merge_base(&oid, repository, &oid_one, &oid_two);
    if (GIT_OK != err) {
        if (GIT_ENOTFOUND == err)
            err = GIT_OK;
        goto cleanup;
    }

    err = git_commit_lookup(&commit, repository, &oid);
    if (GIT_OK != err)
        goto cleanup;

    PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_commit")));
    git2r_commit_init(commit, repo, result);

cleanup:
    if (commit)
        git_commit_free(commit);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (GIT_OK != err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return result;
}
