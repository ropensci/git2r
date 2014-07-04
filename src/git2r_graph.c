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
#include "git2r_error.h"
#include "git2r_oid.h"
#include "git2r_repository.h"

SEXP git2r_graph_ahead_behind(SEXP local, SEXP upstream)
{
    size_t ahead, behind;
    int err;
    SEXP result = R_NilValue;
    SEXP slot;
    git_oid local_oid;
    git_oid upstream_oid;
    git_repository *repository = NULL;

    if (git2r_arg_check_commit(local)
        || git2r_arg_check_commit(upstream))
        error("Invalid arguments to git2r_graph_ahead_behind");

    slot = GET_SLOT(local, Rf_install("repo"));
    repository = git2r_repository_open(slot);
    if (!repository)
        error(git2r_err_invalid_repository);

    slot = GET_SLOT(local, Rf_install("hex"));
    git2r_oid_from_hex_sexp(slot, &local_oid);

    slot = GET_SLOT(upstream, Rf_install("hex"));
    git2r_oid_from_hex_sexp(slot, &upstream_oid);

    err = git_graph_ahead_behind(&ahead, &behind, repository, &local_oid,
                                 &upstream_oid);
    if (err < 0)
        goto cleanup;

    PROTECT(result = allocVector(INTSXP, 2));
    INTEGER(result)[0] = ahead;
    INTEGER(result)[1] = behind;

cleanup:
    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return result;
}

/**
 * Determine if a commit is the descendant of another commit.
 *
 * @param commit
 * @param ancestor
 * @return TRUE or FALSE
 */
SEXP git2r_graph_descendant_of(SEXP commit, SEXP ancestor)
{
    int result;
    SEXP slot;
    git_oid commit_oid;
    git_oid ancestor_oid;
    git_repository *repository = NULL;

    if (git2r_arg_check_commit(commit)
        || git2r_arg_check_commit(ancestor))
        error("Invalid arguments to git2r_graph_descendant_of");

    slot = GET_SLOT(commit, Rf_install("repo"));
    repository = git2r_repository_open(slot);
    if (!repository)
        error(git2r_err_invalid_repository);

    slot = GET_SLOT(commit, Rf_install("hex"));
    git2r_oid_from_hex_sexp(slot, &commit_oid);

    slot = GET_SLOT(ancestor, Rf_install("hex"));
    git2r_oid_from_hex_sexp(slot, &ancestor_oid);

    result = git_graph_descendant_of(repository, &commit_oid, &ancestor_oid);
    git_repository_free(repository);

    if (result < 0)
        error("Error: %s\n", giterr_last()->message);
    if (0 == result)
        return ScalarLogical(0);
    return ScalarLogical(1);
}
