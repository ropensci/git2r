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
 * Checkout
 *
 * @param repository
 * @param oid
 * @return
 */
static int checkout(git_repository *repository, git_oid *oid)
{
    int err = 0;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
    checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;

    return err;
}

/**
 * Checkout commit
 *
 * @param commit S4 class git_commit
 * @return R_NilValue
 */
SEXP checkout_commit(SEXP commit)
{
    int err;
    SEXP slot;
    const char *class_name;
    git_oid oid;
    git_repository *repository = NULL;

    if (check_tag_arg(commit))
        error("Invalid arguments to checkout_commit");

    repository = get_repository(GET_SLOT(commit, Rf_install("repo")));
    if (!repository)
        error(git2r_err_invalid_repository);

    slot = GET_SLOT(commit, Rf_install("hex"));
    err = git_oid_fromstr(&oid, CHAR(STRING_ELT(slot, 0)));
    if (err < 0)
        goto cleanup;

    err = checkout(repository, &oid);

cleanup:
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
SEXP checkout_tag(SEXP tag)
{
    int err;
    SEXP slot;
    const char *class_name;
    git_oid oid;
    git_repository *repository = NULL;

    if (check_tag_arg(tag))
        error("Invalid arguments to checkout_tag");

    repository = get_repository(GET_SLOT(tag, Rf_install("repo")));
    if (!repository)
        error(git2r_err_invalid_repository);

    slot = GET_SLOT(tag, Rf_install("target"));
    err = git_oid_fromstr(&oid, CHAR(STRING_ELT(slot, 0)));
    if (err < 0)
        goto cleanup;

    err = checkout(repository, &oid);

cleanup:
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
SEXP checkout_tree(SEXP tree)
{
    int err;
    SEXP slot;
    const char *class_name;
    git_oid oid;
    git_repository *repository = NULL;

    if (check_tag_arg(tree))
        error("Invalid arguments to checkout_tree");

    repository = get_repository(GET_SLOT(tree, Rf_install("repo")));
    if (!repository)
        error(git2r_err_invalid_repository);

    slot = GET_SLOT(tree, Rf_install("hex"));
    err = git_oid_fromstr(&oid, CHAR(STRING_ELT(slot, 0)));
    if (err < 0)
        goto cleanup;

    err = checkout(repository, &oid);

cleanup:
    if (repository)
        git_repository_free(repository);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return R_NilValue;
}
