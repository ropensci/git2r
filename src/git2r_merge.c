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
#include "buffer.h"

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
 * @param merge_result S4 class git_merge_result
 * @param merge_head The merge head to fast-forward merge
 * @param repository The repository
 * @param name The name of the merge in the reflog
 * @param merger Who is performing the merge
 * @return 0 on success, or error code
 */
static int git2r_fast_forward_merge(
    SEXP merge_result,
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

    SET_SLOT(
        merge_result,
        Rf_install("status"),
        ScalarString(mkChar("Fast-forward")));

    SET_SLOT(
        merge_result,
        Rf_install("conflicts"),
        ScalarLogical(0));

cleanup:
    if (commit)
        git_commit_free(commit);

    if (reference)
        git_reference_free(reference);

    if (tree)
        git_tree_free(tree);

    return err;
}

/**
 * Perform a normal merge
 *
 * @param merge_result S4 class git_merge_result
 * @param merge_heads The merge heads to merge
 * @param n The number of merge heads
 * @param repository The repository
 * @param name The name of the merge in the reflog
 * @param merger Who is performing the merge
 * @param commit_on_success Commit merge commit, if one was created
 * @param merge_opts Merge options
 * @return 0 on success, or error code
 */
static int git2r_normal_merge(
    SEXP merge_result,
    const git_merge_head **merge_heads,
    size_t n,
    git_repository *repository,
    git_signature *merger,
    int commit_on_success,
    const git_merge_options *merge_opts)
{
    int err;
    git_index *index = NULL;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

    err = git_merge(
        repository,
        merge_heads,
        n,
        merge_opts,
        &checkout_opts);
    if (GIT_OK != err)
        goto cleanup;

    err = git_repository_index(&index, repository);
    if (GIT_OK != err)
        goto cleanup;

    if (git_index_has_conflicts(index)) {
        SET_SLOT(merge_result, Rf_install("conflicts"), ScalarLogical(1));
    } else {
        SET_SLOT(merge_result, Rf_install("conflicts"), ScalarLogical(0));
    }

cleanup:
    if (index)
        git_index_free(index);

    return err;
}

/**
 * @param merge_result S4 class git_merge_result
 * @repository The repository
 * @param merge_head The merge head to merge
 * @param n The number of merge heads
 * @param preference The merge preference option (None [0], No
 * Fast-Forward [1] or Only Fast-Forward [2])
 * @param name The name of the merge in the reflog
 * @param merger Who is performing the merge
 * @param commit_on_success Commit merge commit, if one was created
 * during a normal merge
 * @return 0 on success, or error code
 */
static int git2r_merge(
    SEXP merge_result,
    git_repository *repository,
    const git_merge_head **merge_heads,
    size_t n,
    git_merge_preference_t preference,
    const char *name,
    git_signature *merger,
    int commit_on_success)
{
    int err;
    git_merge_analysis_t merge_analysis;
    git_merge_preference_t merge_preference;
    git_merge_options merge_opts = GIT_MERGE_OPTIONS_INIT;

    merge_opts.rename_threshold = 50;
    merge_opts.target_limit = 200;

    err = git_merge_analysis(
        &merge_analysis,
        &merge_preference,
        repository,
        merge_heads,
        n);
    if (GIT_OK != err)
        return err;

    if (merge_analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) {
        SET_SLOT(merge_result,
                 Rf_install("status"),
                 ScalarString(mkChar("Already up-to-date.")));
        return GIT_OK;
    }

    if (GIT_MERGE_PREFERENCE_NONE == preference)
        preference = merge_preference;

    switch (preference) {
    case GIT_MERGE_PREFERENCE_NONE:
        if (merge_analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) {
            if (1 != n) {
                giterr_set_str(
                    GITERR_NONE,
                    "Unable to perform Fast-Forward merge with mith multiple merge heads.");
                return GIT_ERROR;
            }

            err = git2r_fast_forward_merge(
                merge_heads[0],
                repository,
                name,
                merger);
        } else if (merge_analysis & GIT_MERGE_ANALYSIS_NORMAL) {
            err = git2r_normal_merge(
                merge_result,
                merge_heads,
                n,
                repository,
                merger,
                commit_on_success,
                &merge_opts);
        }
        break;
    case GIT_MERGE_PREFERENCE_NO_FASTFORWARD:
        if (merge_analysis & GIT_MERGE_ANALYSIS_NORMAL) {
            err = git2r_normal_merge(
                merge_result,
                merge_heads,
                n,
                repository,
                merger,
                commit_on_success,
                &merge_opts);
        }
        break;
    case GIT_MERGE_PREFERENCE_FASTFORWARD_ONLY:
        if (merge_analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) {
            if (1 != n) {
                giterr_set_str(
                    GITERR_NONE,
                    "Unable to perform Fast-Forward merge with mith multiple merge heads.");
                return GIT_ERROR;
            }

            err = git2r_fast_forward_merge(
                merge_heads[0],
                repository,
                name,
                merger);
        } else {
            giterr_set_str(GITERR_NONE, "Unable to perform Fast-Forward merge.");
            return GIT_ERROR;
        }
        break;
    default:
        giterr_set_str(GITERR_NONE, "Unknown merge option");
        return GIT_ERROR;
    }

    return GIT_OK;
}
