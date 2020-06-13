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
#include "git2r_deprecated.h"
#include "git2r_error.h"
#include "git2r_oid.h"
#include "git2r_repository.h"
#include "git2r_S3.h"

/**
 * Count the number of unique commits between two commit objects
 *
 * @param local The commit for local
 * @param upstream The commit for upstream
 * @return Integer vector of length two with the values ahead and
 * behind.
 */
SEXP attribute_hidden
git2r_graph_ahead_behind(
    SEXP local,
    SEXP upstream)
{
    size_t ahead, behind;
    int error, nprotect = 0;
    SEXP result = R_NilValue;
    SEXP local_repo, local_sha;
    SEXP upstream_repo, upstream_sha;
    git_oid local_oid, upstream_oid;
    git_repository *repository = NULL;

    if (git2r_arg_check_commit(local))
        git2r_error(__func__, NULL, "'local'", git2r_err_commit_arg);
    if (git2r_arg_check_commit(upstream))
        git2r_error(__func__, NULL, "'upstream'", git2r_err_commit_arg);

    local_repo = git2r_get_list_element(local, "repo");
    upstream_repo = git2r_get_list_element(upstream, "repo");
    if (git2r_arg_check_same_repo(local_repo, upstream_repo))
        git2r_error(__func__, NULL, "'local' and 'upstream' not from same repository", NULL);

    repository = git2r_repository_open(local_repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    local_sha = git2r_get_list_element(local, "sha");
    git2r_oid_from_sha_sexp(local_sha, &local_oid);

    upstream_sha = git2r_get_list_element(upstream, "sha");
    git2r_oid_from_sha_sexp(upstream_sha, &upstream_oid);

    error = git_graph_ahead_behind(&ahead, &behind, repository, &local_oid,
                                 &upstream_oid);
    if (error)
        goto cleanup;

    PROTECT(result = Rf_allocVector(INTSXP, 2));
    nprotect++;
    INTEGER(result)[0] = ahead;
    INTEGER(result)[1] = behind;

cleanup:
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Determine if a commit is the descendant of another commit.
 *
 * @param commit A commit.
 * @param ancestor A potential ancestor commit.
 * @return TRUE or FALSE
 */
SEXP attribute_hidden
git2r_graph_descendant_of(
    SEXP commit,
    SEXP ancestor)
{
    int error, descendant_of = 0;
    SEXP commit_repo, commit_sha;
    SEXP ancestor_repo, ancestor_sha;
    git_oid commit_oid, ancestor_oid;
    git_repository *repository = NULL;

    if (git2r_arg_check_commit(commit))
        git2r_error(__func__, NULL, "'commit'", git2r_err_commit_arg);
    if (git2r_arg_check_commit(ancestor))
        git2r_error(__func__, NULL, "'ancestor'", git2r_err_commit_arg);

    commit_repo = git2r_get_list_element(commit, "repo");
    ancestor_repo = git2r_get_list_element(ancestor, "repo");
    if (git2r_arg_check_same_repo(commit_repo, ancestor_repo))
        git2r_error(__func__, NULL, "'commit' and 'ancestor' not from same repository", NULL);

    repository = git2r_repository_open(commit_repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    commit_sha = git2r_get_list_element(commit, "sha");
    git2r_oid_from_sha_sexp(commit_sha, &commit_oid);

    ancestor_sha = git2r_get_list_element(ancestor, "sha");
    git2r_oid_from_sha_sexp(ancestor_sha, &ancestor_oid);

    error = git_graph_descendant_of(repository, &commit_oid, &ancestor_oid);
    if (0 > error || 1 < error)
        goto cleanup;
    descendant_of = error;
    error = 0;

cleanup:
    git_repository_free(repository);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return Rf_ScalarLogical(descendant_of);
}
