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
    SEXP hex;
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

    hex = GET_SLOT(one, Rf_install("hex"));
    err = git_oid_fromstr(&oid_one, CHAR(STRING_ELT(hex, 0)));
    if (GIT_OK != err)
        goto cleanup;

    hex = GET_SLOT(two, Rf_install("hex"));
    err = git_oid_fromstr(&oid_two, CHAR(STRING_ELT(hex, 0)));
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

/**
 * Perform a fast-forward merge
 *
 * @param merge_head The merge head to fast-forward merge
 * @param repository The repository
 * @param name The name of the merge in the reflog
 * @param merger Who is performing the merge
 * @return 0 on success, or error code
 */
static int git2r_fast_forward_merge(
    const git_merge_head *merge_head,
    git_repository *repository,
    const char *name,
    git_signature *merger)
{
    int err;
    const git_oid *oid;
    git_buf buf = GIT_BUF_INIT;
    git_commit *commit = NULL;
    git_tree *tree = NULL;
    git_reference *reference = NULL;
    git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;

    oid = git_merge_head_id(merge_head);
    err = git_commit_lookup(&commit, repository, oid);
    if (GIT_OK != err)
        goto cleanup;

    err = git_commit_tree(&tree, commit);
    if (GIT_OK != err)
        goto cleanup;

    opts.checkout_strategy = GIT_CHECKOUT_SAFE;
    err = git_checkout_tree(repository, (git_object*)tree, &opts);
    if (GIT_OK != err)
        goto cleanup;

    err = git_repository_head(&reference, repository);
    if (GIT_OK != err) {
        if (GIT_ENOTFOUND != err)
            goto cleanup;
    }

    err = git_buf_printf(&buf, "merge %s: Fast-forward", name);
    if (GIT_OK != err)
        goto cleanup;

    if (GIT_ENOTFOUND == err) {
        err = git_reference_create(
            &reference,
            repository,
            "HEAD",
            git_commit_id(commit),
            0, /* force */
            merger,
            buf.ptr);
    } else {
        git_reference *target_ref = NULL;

        err = git_reference_set_target(
            &target_ref,
            reference,
            git_commit_id(commit),
            merger,
            buf.ptr);

        if (target_ref)
            git_reference_free(target_ref);
    }

    git_buf_free(&buf);

cleanup:
    if (commit)
        git_commit_free(commit);

    if (reference)
        git_reference_free(reference);

    if (tree)
        git_tree_free(tree);

    return err;
}
