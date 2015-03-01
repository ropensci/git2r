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
#include "refs.h"

#include "git2r_arg.h"
#include "git2r_checkout.h"
#include "git2r_error.h"
#include "git2r_repository.h"
#include "git2r_signature.h"

/**
 * Checkout tree
 *
 * @param repo S4 class git_repository
 * @param revision The revision string, see
 * http://git-scm.com/docs/git-rev-parse.html#_specifying_revisions
 * @param force Using checkout strategy GIT_CHECKOUT_SAFE (force =
 *        FALSE) or GIT_CHECKOUT_FORCE (force = TRUE).
 * @return R_NilValue
 */
SEXP git2r_checkout_tree(SEXP repo, SEXP revision, SEXP force)
{
    int err;
    git_repository *repository = NULL;
    git_object *treeish = NULL;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

    if (GIT_OK != git2r_arg_check_string(revision))
        git2r_error(git2r_err_string_arg, __func__, "revision");
    if (GIT_OK != git2r_arg_check_logical(force))
        git2r_error(git2r_err_logical_arg, __func__, "force");

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git_revparse_single(&treeish, repository, CHAR(STRING_ELT(revision, 0)));
    if (GIT_OK != err)
        goto cleanup;

    switch (git_object_type(treeish)) {
    case GIT_OBJ_COMMIT:
    case GIT_OBJ_TAG:
    case GIT_OBJ_TREE:
        err = GIT_OK;
        break;
    default:
        giterr_set_str(GITERR_NONE, git2r_err_checkout_tree);
        err = GIT_ERROR;
        break;
    }
    if (GIT_OK != err)
        goto cleanup;

    if (LOGICAL(force)[0])
        checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE;
    else
        checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;

    err = git_checkout_tree(repository, treeish, &checkout_opts);

cleanup:
    if (treeish)
        git_object_free(treeish);

    if (repository)
        git_repository_free(repository);

    if (GIT_OK != err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

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
    git_signature *signature = NULL;
    git_commit *treeish = NULL;
    git_repository *repository = NULL;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

    if (GIT_OK != git2r_arg_check_tag(tag))
        git2r_error(git2r_err_tag_arg, __func__, "tag");
    if (GIT_OK != git2r_arg_check_logical(force))
        git2r_error(git2r_err_logical_arg, __func__, "force");
    if (GIT_OK != git2r_arg_check_string(msg))
        git2r_error(git2r_err_string_arg, __func__, "msg");
    if (GIT_OK != git2r_arg_check_signature(who))
        git2r_error(git2r_err_signature_arg, __func__, "who");

    repository = git2r_repository_open(GET_SLOT(tag, Rf_install("repo")));
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    slot = GET_SLOT(tag, Rf_install("target"));
    err = git_oid_fromstr(&oid, CHAR(STRING_ELT(slot, 0)));
    if (GIT_OK != err)
        goto cleanup;

    err = git_commit_lookup(&treeish, repository, &oid);
    if (GIT_OK != err)
        goto cleanup;

    err = git2r_signature_from_arg(&signature, who);
    if (GIT_OK != err)
        goto cleanup;

    err = git_repository_set_head_detached(
        repository,
        git_commit_id(treeish),
        signature,
        CHAR(STRING_ELT(msg, 0)));
    if (GIT_OK != err)
        goto cleanup;

    if (LOGICAL(force)[0])
        checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE;
    else
        checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE_CREATE;
    err = git_checkout_head(repository, &checkout_opts);

cleanup:
    if (signature)
        git_signature_free(signature);

    if (treeish)
        git_commit_free(treeish);

    if (repository)
        git_repository_free(repository);

    if (GIT_OK != err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return R_NilValue;
}
