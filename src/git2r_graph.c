/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2018 The git2r contributors
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
#include "git2r_error.h"
#include "git2r_oid.h"
#include "git2r_repository.h"

/**
 * Count the number of unique commits between two commit objects
 *
 * @param local The commit for local
 * @param upstream The commit for upstream
 * @return Integer vector of length two with the values ahead and
 * behind.
 */
SEXP git2r_graph_ahead_behind(SEXP local, SEXP upstream)
{
    size_t ahead, behind;
    int err, nprotect = 0;
    SEXP result = R_NilValue;
    SEXP local_path, local_repo, local_sha;
    SEXP upstream_path, upstream_repo, upstream_sha;
    git_oid local_oid, upstream_oid;
    git_repository *repository = NULL;

    if (git2r_arg_check_commit(local))
        git2r_error(__func__, NULL, "'local'", git2r_err_commit_arg);
    if (git2r_arg_check_commit(upstream))
        git2r_error(__func__, NULL, "'upstream'", git2r_err_commit_arg);

    local_repo = GET_SLOT(local, Rf_install("repo"));
    upstream_repo = GET_SLOT(upstream, Rf_install("repo"));
    if (git2r_arg_check_same_repo(local_repo, upstream_repo))
        git2r_error(__func__, NULL, "'local' and 'upstream' not from same repository", NULL);

    repository = git2r_repository_open(local_repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    local_sha = GET_SLOT(local, Rf_install("sha"));
    git2r_oid_from_sha_sexp(local_sha, &local_oid);

    upstream_sha = GET_SLOT(upstream, Rf_install("sha"));
    git2r_oid_from_sha_sexp(upstream_sha, &upstream_oid);

    err = git_graph_ahead_behind(&ahead, &behind, repository, &local_oid,
                                 &upstream_oid);
    if (err)
        goto cleanup;

    PROTECT(result = allocVector(INTSXP, 2));
    nprotect++;
    INTEGER(result)[0] = ahead;
    INTEGER(result)[1] = behind;

cleanup:
    if (repository)
        git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return result;
}

/**
 * Determine if a commit is the descendant of another commit.
 *
 * @param commit A commit.
 * @param ancestor A potential ancestor commit.
 * @return TRUE or FALSE
 */
SEXP git2r_graph_descendant_of(SEXP commit, SEXP ancestor)
{
    int err, descendant_of = 0;
    SEXP commit_path, commit_repo, commit_sha;
    SEXP ancestor_path, ancestor_repo, ancestor_sha;
    git_oid commit_oid, ancestor_oid;
    git_repository *repository = NULL;

    if (git2r_arg_check_commit(commit))
        git2r_error(__func__, NULL, "'commit'", git2r_err_commit_arg);
    if (git2r_arg_check_commit(ancestor))
        git2r_error(__func__, NULL, "'ancestor'", git2r_err_commit_arg);

    commit_repo = GET_SLOT(commit, Rf_install("repo"));
    ancestor_repo = GET_SLOT(ancestor, Rf_install("repo"));
    if (git2r_arg_check_same_repo(commit_repo, ancestor_repo))
        git2r_error(__func__, NULL, "'commit' and 'ancestor' not from same repository", NULL);

    repository = git2r_repository_open(commit_repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    commit_sha = GET_SLOT(commit, Rf_install("sha"));
    git2r_oid_from_sha_sexp(commit_sha, &commit_oid);

    ancestor_sha = GET_SLOT(ancestor, Rf_install("sha"));
    git2r_oid_from_sha_sexp(ancestor_sha, &ancestor_oid);

    err = git_graph_descendant_of(repository, &commit_oid, &ancestor_oid);
    if (0 > err || 1 < err)
        goto cleanup;
    descendant_of = err;
    err = 0;

cleanup:
    if (repository)
        git_repository_free(repository);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return ScalarLogical(descendant_of);
}
