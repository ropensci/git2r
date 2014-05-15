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
#include "git2r_checkout.h"
#include "git2r_error.h"
#include "git2r_repository.h"

/**
 * Checkout commit
 *
 * @param commit S4 class git_commit
 * @param force
 * @return R_NilValue
 */
SEXP git2r_checkout_commit(SEXP commit, SEXP force)
{
    int err;
    SEXP hex;
    git_oid oid;
    git_commit *treeish = NULL;
    git_repository *repository = NULL;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

    if (check_commit_arg(commit) || check_logical_arg(force))
        error("Invalid arguments to checkout_commit");

    repository = get_repository(GET_SLOT(commit, Rf_install("repo")));
    if (!repository)
        error(git2r_err_invalid_repository);

    hex = GET_SLOT(commit, Rf_install("hex"));
    err = git_oid_fromstr(&oid, CHAR(STRING_ELT(hex, 0)));
    if (err < 0)
        goto cleanup;

    err = git_commit_lookup(&treeish, repository, &oid);
    if (err < 0)
        goto cleanup;

    err = git_repository_set_head_detached(repository, git_commit_id(treeish),
                                           NULL, NULL);
    if (err < 0)
        goto cleanup;

    if (LOGICAL(force)[0])
        checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE;
    else
        checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE_CREATE;
    err = git_checkout_head(repository, &checkout_opts);
    if (err < 0)
        goto cleanup;

cleanup:
    if (treeish)
        git_commit_free(treeish);

    if (repository)
        git_repository_free(repository);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return R_NilValue;
}

/**
 * Checkout tag
 *
 * @param tag S4 class git_tag
 * @return R_NilValue
 */
SEXP git2r_checkout_tag(SEXP tag, SEXP force)
{
    int err;
    SEXP slot;
    git_oid oid;
    git_commit *treeish = NULL;
    git_repository *repository = NULL;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

    if (check_tag_arg(tag))
        error("Invalid arguments to checkout_tag");

    repository = get_repository(GET_SLOT(tag, Rf_install("repo")));
    if (!repository)
        error(git2r_err_invalid_repository);

    slot = GET_SLOT(tag, Rf_install("target"));
    err = git_oid_fromstr(&oid, CHAR(STRING_ELT(slot, 0)));
    if (err < 0)
        goto cleanup;

    err = git_commit_lookup(&treeish, repository, &oid);
    if (err < 0)
        goto cleanup;

    err = git_repository_set_head_detached(repository, git_commit_id(treeish),
                                           NULL, NULL);
    if (err < 0)
        goto cleanup;

    if (LOGICAL(force)[0])
        checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE;
    else
        checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE_CREATE;
    err = git_checkout_head(repository, &checkout_opts);
    if (err < 0)
        goto cleanup;

cleanup:
    if (treeish)
        git_commit_free(treeish);

    if (repository)
        git_repository_free(repository);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return R_NilValue;
}

/**
 * Checkout tree
 *
 * @param tree S4 class git_tree
 * @return R_NilValue
 */
SEXP git2r_checkout_tree(SEXP tree, SEXP force)
{
    int err;
    SEXP slot;
    git_oid oid;
    git_repository *repository = NULL;

    if (check_tree_arg(tree))
        error("Invalid arguments to checkout_tree");

    repository = get_repository(GET_SLOT(tree, Rf_install("repo")));
    if (!repository)
        error(git2r_err_invalid_repository);

    slot = GET_SLOT(tree, Rf_install("hex"));
    err = git_oid_fromstr(&oid, CHAR(STRING_ELT(slot, 0)));
    if (err < 0)
        goto cleanup;

cleanup:
    if (repository)
        git_repository_free(repository);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return R_NilValue;
}
