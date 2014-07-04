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
#include "refs.h"
#include "git2r_checkout.h"
#include "git2r_error.h"
#include "git2r_repository.h"

/**
 * Checkout branch
 *
 * @param branch S4 class git_branch
 * @param force Using checkout strategy GIT_CHECKOUT_SAFE_CREATE (force = TRUE)
 *        or GIT_CHECKOUT_FORCE (force = FALSE).
 * @param msg The one line long message to be appended to the reflog
 * @param who The identity that will used to populate the reflog entry
 * @return R_NilValue
 */
SEXP git2r_checkout_branch(
    SEXP branch,
    SEXP force,
    SEXP msg,
    SEXP who)
{
    int err;
    SEXP branch_name;
    git_buf ref_name = GIT_BUF_INIT;
    git_oid oid;
    const char *message = NULL;
    git_signature *signature = NULL;
    git_reference *reference = NULL;
    git_repository *repository = NULL;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

    if (git2r_arg_check_branch(branch)
        || git2r_arg_check_logical(force)
        || git2r_arg_check_signature(who))
        error("Invalid arguments to git2r_checkout_branch");

    if (R_NilValue != msg) {
        if (git2r_error_check_string_arg(msg))
            error("Invalid arguments to git2r_checkout_branch");
        message = CHAR(STRING_ELT(msg, 0));
    }

    repository = git2r_repository_open(GET_SLOT(branch, Rf_install("repo")));
    if (!repository)
        error(git2r_err_invalid_repository);

    branch_name = GET_SLOT(branch, Rf_install("name"));
    err = git_buf_joinpath(
        &ref_name,
        GIT_REFS_HEADS_DIR,
        CHAR(STRING_ELT(branch_name, 0)));
    if (err < 0)
        goto cleanup;

    err = git2r_signature_from_arg(&signature, who);
    if (err < 0)
        goto cleanup;

    err = git_repository_set_head(
        repository,
        ref_name.ptr,
        signature,
        message);
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
    git_buf_free(&ref_name);

    if (signature)
        git_signature_free(signature);

    if (reference)
        git_reference_free(reference);

    if (repository)
        git_repository_free(repository);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return R_NilValue;
}

/**
 * Checkout commit
 *
 * @param commit S4 class git_commit
 * @param force Using checkout strategy GIT_CHECKOUT_SAFE_CREATE (force = FALSE)
 *        or GIT_CHECKOUT_FORCE (force = TRUE).
 * @param msg The one line long message to be appended to the reflog
 * @param who The identity that will used to populate the reflog entry
 * @return R_NilValue
 */
SEXP git2r_checkout_commit(
    SEXP commit,
    SEXP force,
    SEXP msg,
    SEXP who)
{
    int err;
    SEXP hex;
    git_oid oid;
    const char *message = NULL;
    git_signature *signature = NULL;
    git_commit *treeish = NULL;
    git_repository *repository = NULL;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

    if (git2r_arg_check_commit(commit)
        || git2r_arg_check_logical(force)
        || git2r_arg_check_signature(who))
        error("Invalid arguments to git2r_checkout_commit");

    if (R_NilValue != msg) {
        if (git2r_error_check_string_arg(msg))
            error("Invalid arguments to git2r_checkout_commit");
        message = CHAR(STRING_ELT(msg, 0));
    }

    repository = git2r_repository_open(GET_SLOT(commit, Rf_install("repo")));
    if (!repository)
        error(git2r_err_invalid_repository);

    hex = GET_SLOT(commit, Rf_install("hex"));
    err = git_oid_fromstr(&oid, CHAR(STRING_ELT(hex, 0)));
    if (err < 0)
        goto cleanup;

    err = git_commit_lookup(&treeish, repository, &oid);
    if (err < 0)
        goto cleanup;

    err = git2r_signature_from_arg(&signature, who);
    if (err < 0)
        goto cleanup;

    err = git_repository_set_head_detached(
        repository,
        git_commit_id(treeish),
        signature,
        message);
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
    if (signature)
        git_signature_free(signature);

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
 * @param force Using checkout strategy GIT_CHECKOUT_SAFE_CREATE (force = FALSE)
 *        or GIT_CHECKOUT_FORCE (force = TRUE).
 * @param msg The one line long message to be appended to the reflog
 * @param who The identity that will used to populate the reflog entry
 * @return R_NilValue
 */
SEXP git2r_checkout_tag(
    SEXP tag,
    SEXP force,
    SEXP msg,
    SEXP who)
{
    int err;
    SEXP slot;
    git_oid oid;
    const char *message = NULL;
    git_signature *signature = NULL;
    git_commit *treeish = NULL;
    git_repository *repository = NULL;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

    if (git2r_error_check_tag_arg(tag)
        || git2r_arg_check_logical(force)
        || git2r_arg_check_signature(who))
        error("Invalid arguments to git2r_checkout_tag");

    if (R_NilValue != msg) {
        if (git2r_error_check_string_arg(msg))
            error("Invalid arguments to git2r_checkout_tag");
        message = CHAR(STRING_ELT(msg, 0));
    }

    repository = git2r_repository_open(GET_SLOT(tag, Rf_install("repo")));
    if (!repository)
        error(git2r_err_invalid_repository);

    slot = GET_SLOT(tag, Rf_install("target"));
    err = git_oid_fromstr(&oid, CHAR(STRING_ELT(slot, 0)));
    if (err < 0)
        goto cleanup;

    err = git_commit_lookup(&treeish, repository, &oid);
    if (err < 0)
        goto cleanup;

    err = git2r_signature_from_arg(&signature, who);
    if (err < 0)
        goto cleanup;

    err = git_repository_set_head_detached(
        repository,
        git_commit_id(treeish),
        signature,
        message);
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
    if (signature)
        git_signature_free(signature);

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
 * @param force Using checkout strategy GIT_CHECKOUT_SAFE_CREATE (force = FALSE)
 *        or GIT_CHECKOUT_FORCE (force = TRUE).
 * @param msg The one line long message to be appended to the reflog
 * @param who The identity that will used to populate the reflog entry
 * @return R_NilValue
 */
SEXP git2r_checkout_tree(
    SEXP tree,
    SEXP force,
    SEXP msg,
    SEXP who)
{
    int err;
    SEXP slot;
    git_oid oid;
    const char *message = NULL;
    git_signature *signature = NULL;
    git_repository *repository = NULL;

    if (git2r_error_check_tree_arg(tree)
        || git2r_arg_check_logical(force)
        || git2r_arg_check_signature(who))
        error("Invalid arguments to git2r_checkout_tree");

    if (R_NilValue != msg) {
        if (git2r_error_check_string_arg(msg))
            error("Invalid arguments to git2r_checkout_tree");
        message = CHAR(STRING_ELT(msg, 0));
    }

    repository = git2r_repository_open(GET_SLOT(tree, Rf_install("repo")));
    if (!repository)
        error(git2r_err_invalid_repository);

    slot = GET_SLOT(tree, Rf_install("hex"));
    err = git_oid_fromstr(&oid, CHAR(STRING_ELT(slot, 0)));
    if (err < 0)
        goto cleanup;

cleanup:
    if (signature)
        git_signature_free(signature);

    if (repository)
        git_repository_free(repository);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return R_NilValue;
}
