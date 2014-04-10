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

#include "git2r_checkout.h"
#include "git2r_error.h"
#include "git2r_repository.h"

/**
 * Checkout
 *
 * @param repo S4 class git_repository
 * @param treeish
 * @return R_NilValue
 */
SEXP checkout(SEXP repo, SEXP treeish)
{
    enum CHECKOUT_ACTION {
        CHECKOUT_COMMIT,
        CHECKOUT_TAG,
        CHECKOUT_TREE,
        CHECKOUT_HEAD
    } checkout_action;

    int err;
    git_repository *repository = NULL;
    git_checkout_opts checkout_opts = GIT_CHECKOUT_OPTS_INIT;
    checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;

    /* Check arguments to checkout */
    if (R_NilValue == repo || R_NilValue == treeish)
        error(git2r_err_invalid_checkout_args);

    /* Determine checkout strategy */
    if(S4SXP == TYPEOF(treeish)) {
        const char *class_name = CHAR(STRING_ELT(getAttrib(treeish, R_ClassSymbol), 0));

        if (0 == strcmp(class_name, "git_commit"))
            checkout_action = CHECKOUT_COMMIT;
        else if(0 == strcmp(class_name, "git_tag"))
            checkout_action = CHECKOUT_TAG;
        else if(0 == strcmp(class_name, "git_tree"))
            checkout_action = CHECKOUT_TREE;
        else
            error(git2r_err_invalid_checkout_args);
    } else if (isString(treeish)
               && 1 == length(treeish)
               && 0 == strcmp(CHAR(STRING_ELT(treeish, 0)), "HEAD")) {
        checkout_action = CHECKOUT_HEAD;
    } else {
        error(git2r_err_invalid_checkout_args);
    }

    repository = get_repository(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    switch (checkout_action) {
    case CHECKOUT_COMMIT:
        /* :TODO:FIX: */
        break;
    case CHECKOUT_TAG:
        /* :TODO:FIX: */
        break;
    case CHECKOUT_TREE:
        /* :TODO:FIX: */
        break;
    case CHECKOUT_HEAD:
        /* :TODO:FIX: */
        break;
    default:
        break;
    }

cleanup:
    if (repository)
        git_repository_free(repository);

    if (err < 0) {
        const git_error *e = giterr_last();
        error("Error %d: %s\n", e->klass, e->message);
    }

    return R_NilValue;
}
