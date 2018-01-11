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
#include "buffer.h"

#include "git2r_arg.h"
#include "git2r_commit.h"
#include "git2r_error.h"
#include "git2r_merge.h"
#include "git2r_repository.h"
#include "git2r_signature.h"

int git2r_commit_create(
    git_oid *out,
    git_repository *repository,
    git_index *index,
    const char *message,
    git_signature *author,
    git_signature *committer);

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
    SEXP repo_one, repo_two;
    SEXP sha;
    git_oid oid, oid_one, oid_two;
    git_commit *commit = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_commit(one))
        git2r_error(__func__, NULL, "'one'", git2r_err_commit_arg);
    if (git2r_arg_check_commit(two))
        git2r_error(__func__, NULL, "'two'", git2r_err_commit_arg);

    repo_one = GET_SLOT(one, Rf_install("repo"));
    repo_two = GET_SLOT(two, Rf_install("repo"));
    if (git2r_arg_check_same_repo(repo_one, repo_two))
        git2r_error(__func__, NULL, "'one' and 'two' not from same repository", NULL);

    repository = git2r_repository_open(repo_one);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    sha = GET_SLOT(one, Rf_install("sha"));
    err = git_oid_fromstr(&oid_one, CHAR(STRING_ELT(sha, 0)));
    if (err)
        goto cleanup;

    sha = GET_SLOT(two, Rf_install("sha"));
    err = git_oid_fromstr(&oid_two, CHAR(STRING_ELT(sha, 0)));
    if (err)
        goto cleanup;

    err = git_merge_base(&oid, repository, &oid_one, &oid_two);
    if (err) {
        if (GIT_ENOTFOUND == err)
            err = GIT_OK;
        goto cleanup;
    }

    err = git_commit_lookup(&commit, repository, &oid);
    if (err)
        goto cleanup;

    PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_commit")));
    git2r_commit_init(commit, repo_one, result);

cleanup:
    if (commit)
        git_commit_free(commit);

    if (repository)
        git_repository_free(repository);

    if (!isNull(result))
        UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return result;
}

/**
 * Perform a fast-forward merge
 *
 * @param merge_result S4 class git_merge_result
 * @param merge_head The merge head to fast-forward merge
 * @param repository The repository
 * @param log_message First part of the one line long message in the reflog
 * @return 0 on success, or error code
 */
static int git2r_fast_forward_merge(
    SEXP merge_result,
    const git_annotated_commit *merge_head,
    git_repository *repository,
    const char *log_message)
{
    int err;
    const git_oid *oid;
    git_buf buf = GIT_BUF_INIT;
    git_commit *commit = NULL;
    git_tree *tree = NULL;
    git_reference *reference = NULL;
    git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;

    oid = git_annotated_commit_id(merge_head);
    err = git_commit_lookup(&commit, repository, oid);
    if (err)
        goto cleanup;

    err = git_commit_tree(&tree, commit);
    if (err)
        goto cleanup;

    opts.checkout_strategy = GIT_CHECKOUT_SAFE;
    err = git_checkout_tree(repository, (git_object*)tree, &opts);
    if (err)
        goto cleanup;

    err = git_repository_head(&reference, repository);
    if (err) {
        if (GIT_ENOTFOUND != err)
            goto cleanup;
    }

    err = git_buf_printf(&buf, "%s: Fast-forward", log_message);
    if (err)
        goto cleanup;

    if (GIT_ENOTFOUND == err) {
        err = git_reference_create(
            &reference,
            repository,
            "HEAD",
            git_commit_id(commit),
            0, /* force */
            buf.ptr);
    } else {
        git_reference *target_ref = NULL;

        err = git_reference_set_target(
            &target_ref,
            reference,
            git_commit_id(commit),
            buf.ptr);

        if (target_ref)
            git_reference_free(target_ref);
    }

    SET_SLOT(
        merge_result,
        Rf_install("fast_forward"),
        ScalarLogical(1));

    SET_SLOT(
        merge_result,
        Rf_install("conflicts"),
        ScalarLogical(0));

cleanup:
    git_buf_free(&buf);

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
 * @param message The commit message of the merge
 * @param merger Who is performing the merge
 * @param commit_on_success Commit merge commit, if one was created
 * @param merge_opts Merge options
 * @return 0 on success, or error code
 */
static int git2r_normal_merge(
    SEXP merge_result,
    const git_annotated_commit **merge_heads,
    size_t n,
    git_repository *repository,
    const char *message,
    git_signature *merger,
    int commit_on_success,
    const git_checkout_options *checkout_opts,
    const git_merge_options *merge_opts)
{
    int err;
    git_commit *commit = NULL;
    git_index *index = NULL;

    SET_SLOT(merge_result, Rf_install("fast_forward"), ScalarLogical(0));

    err = git_merge(
        repository,
        merge_heads,
        n,
        merge_opts,
        checkout_opts);
    if (err)
        goto cleanup;

    err = git_repository_index(&index, repository);
    if (err)
        goto cleanup;

    if (git_index_has_conflicts(index)) {
        SET_SLOT(merge_result, Rf_install("conflicts"), ScalarLogical(1));
    } else {
        SET_SLOT(merge_result, Rf_install("conflicts"), ScalarLogical(0));

        if (commit_on_success) {
            char sha[GIT_OID_HEXSZ + 1];
            git_oid oid;
            SEXP s_sha = Rf_install("sha");

            err = git2r_commit_create(
                &oid,
                repository,
                index,
                message,
                merger,
                merger);
            if (err)
                goto cleanup;

            git_oid_fmt(sha, &oid);
            sha[GIT_OID_HEXSZ] = '\0';
            SET_SLOT(merge_result, s_sha, mkString(sha));
        }
    }

cleanup:
    if (commit)
        git_commit_free(commit);

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
    const git_annotated_commit **merge_heads,
    size_t n,
    git_merge_preference_t preference,
    const char *name,
    git_signature *merger,
    int commit_on_success)
{
    int err;
    git_merge_analysis_t merge_analysis;
    git_merge_preference_t merge_preference;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
    git_merge_options merge_opts = GIT_MERGE_OPTIONS_INIT;

    merge_opts.rename_threshold = 50;
    merge_opts.target_limit = 200;

    checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;

    err = git_merge_analysis(
        &merge_analysis,
        &merge_preference,
        repository,
        merge_heads,
        n);
    if (err)
        return err;

    if (merge_analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) {
        SET_SLOT(merge_result,
                 Rf_install("up_to_date"),
                 ScalarLogical(1));
        return GIT_OK;
    } else {
        SET_SLOT(merge_result,
                 Rf_install("up_to_date"),
                 ScalarLogical(0));
    }

    if (GIT_MERGE_PREFERENCE_NONE == preference)
        preference = merge_preference;

    switch (preference) {
    case GIT_MERGE_PREFERENCE_NONE:
        if (merge_analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) {
            if (1 != n) {
                giterr_set_str(
                    GITERR_NONE,
                    "Unable to perform Fast-Forward merge "
                    "with mith multiple merge heads.");
                return GIT_ERROR;
            }

            err = git2r_fast_forward_merge(
                merge_result,
                merge_heads[0],
                repository,
                name);
        } else if (merge_analysis & GIT_MERGE_ANALYSIS_NORMAL) {
            err = git2r_normal_merge(
                merge_result,
                merge_heads,
                n,
                repository,
                name,
                merger,
                commit_on_success,
                &checkout_opts,
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
                name,
                merger,
                commit_on_success,
                &checkout_opts,
                &merge_opts);
        }
        break;
    case GIT_MERGE_PREFERENCE_FASTFORWARD_ONLY:
        if (merge_analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) {
            if (1 != n) {
                giterr_set_str(
                    GITERR_NONE,
                    "Unable to perform Fast-Forward merge "
                    "with mith multiple merge heads.");
                return GIT_ERROR;
            }

            err = git2r_fast_forward_merge(
                merge_result,
                merge_heads[0],
                repository,
                name);
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

/**
 * Free each git_annotated_commit in merge_heads and free memory of
 * merge_heads.
 *
 * @param merge_heads The vector of git_merge_head.
 * @param count The number of merge heads.
 * @return void
 */
static void git2r_merge_heads_free(git_annotated_commit **merge_heads, size_t n)
{
    if (merge_heads) {
        size_t i = 0;

        for (; i < n; i++) {
            if (merge_heads[i])
                git_annotated_commit_free(merge_heads[i]);
        }

        free(merge_heads);
    }
}

/**
 * Merge branch into HEAD
 *
 * @param branch S4 class git_branch to merge into HEAD.
 * @param merger Who is performing the merge
 * @param commit_on_success Commit merge commit, if one was created
 * during a normal merge
 * @return S4 class git_merge_result
 */
SEXP git2r_merge_branch(SEXP branch, SEXP merger, SEXP commit_on_success)
{
    int err;
    SEXP result = R_NilValue;
    const char *name;
    git_buf buf = GIT_BUF_INIT;
    git_branch_t type;
    git_annotated_commit **merge_heads = NULL;
    git_reference *reference = NULL;
    git_repository *repository = NULL;
    git_signature *who = NULL;

    if (git2r_arg_check_branch(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_branch_arg);
    if (git2r_arg_check_logical(commit_on_success))
        git2r_error(__func__, NULL, "'commit_on_success'", git2r_err_logical_arg);
    if (git2r_arg_check_signature(merger))
        git2r_error(__func__, NULL, "'merger'", git2r_err_signature_arg);

    err = git2r_signature_from_arg(&who, merger);
    if (err)
        goto cleanup;

    repository = git2r_repository_open(GET_SLOT(branch, Rf_install("repo")));
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    name = CHAR(STRING_ELT(GET_SLOT(branch, Rf_install("name")), 0));
    type = INTEGER(GET_SLOT(branch, Rf_install("type")))[0];
    err = git_branch_lookup(&reference, repository, name, type);
    if (err)
        goto cleanup;

    merge_heads = calloc(1, sizeof(git_annotated_commit*));
    if (NULL == merge_heads) {
        giterr_set_str(GITERR_NONE, git2r_err_alloc_memory_buffer);
        goto cleanup;
    }

    err = git_annotated_commit_from_ref(
        &(merge_heads[0]),
        repository,
        reference);
    if (err)
        goto cleanup;

    err = git_buf_printf(&buf, "merge %s", name);
    if (err)
        goto cleanup;

    PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_merge_result")));
    err = git2r_merge(
        result,
        repository,
        (const git_annotated_commit **)merge_heads,
        1,
        GIT_MERGE_PREFERENCE_NONE,
        buf.ptr,
        who,
        LOGICAL(commit_on_success)[0]);

cleanup:
    git_buf_free(&buf);

    if (who)
        git_signature_free(who);

    if (merge_heads)
        git2r_merge_heads_free(merge_heads, 1);

    if (reference)
        git_reference_free(reference);

    if (repository)
        git_repository_free(repository);

    if (!isNull(result))
        UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return result;
}

/**
 * Create and populate a vector of git_annotated_commit objects from
 * the given fetch head data.
 *
 * @param out Pointer the vector of git_annotated_commit objects.
 * @param repository The repository.
 * @param fetch_heads List of S4 class git_fetch_head objects.
 * @param n Length of fetch_heads list.
 * @return 0 on success, or error code
 */
static int git2r_merge_heads_from_fetch_heads(
    git_annotated_commit ***merge_heads,
    git_repository *repository,
    SEXP fetch_heads,
    size_t n)
{
    int err = GIT_OK;
    size_t i;

    *merge_heads = calloc(n, sizeof(git_annotated_commit*));
    if (!(*merge_heads)) {
        giterr_set_str(GITERR_NONE, git2r_err_alloc_memory_buffer);
        return GIT_ERROR;
    }

    for (i = 0; i < n; i++) {
        int err;
        git_oid oid;
        SEXP fh = VECTOR_ELT(fetch_heads, i);

        err = git_oid_fromstr(
            &oid,
            CHAR(STRING_ELT(GET_SLOT(fh, Rf_install("sha")), 0)));
        if (err)
            goto cleanup;

        err = git_annotated_commit_from_fetchhead(
            &((*merge_heads)[i]),
            repository,
            CHAR(STRING_ELT(GET_SLOT(fh, Rf_install("ref_name")), 0)),
            CHAR(STRING_ELT(GET_SLOT(fh, Rf_install("remote_url")), 0)),
            &oid);
        if (err)
            goto cleanup;
    }

cleanup:
    if (err) {
        if (*merge_heads)
            git2r_merge_heads_free(*merge_heads, n);
        *merge_heads = NULL;
    }

    return err;
}

/**
 * Merge the given fetch head data into HEAD
 *
 * @param fetch_heads List of S4 class git_fetch_head objects.
 * @param merger Who made the merge, if the merge is non-fast forward
 * merge that creates a merge commit.
 * @return List of git_annotated_commit objects.
 */
SEXP git2r_merge_fetch_heads(SEXP fetch_heads, SEXP merger)
{
    int err;
    size_t n;
    SEXP result = R_NilValue;
    git_annotated_commit **merge_heads = NULL;
    git_repository *repository = NULL;
    git_signature *who = NULL;

    if (git2r_arg_check_fetch_heads(fetch_heads))
        git2r_error(__func__, NULL, "'fetch_heads'", git2r_err_fetch_heads_arg);
    if (git2r_arg_check_signature(merger))
        git2r_error(__func__, NULL, "'merger'", git2r_err_signature_arg);

    err = git2r_signature_from_arg(&who, merger);
    if (err)
        goto cleanup;

    n = LENGTH(fetch_heads);
    if (n) {
        SEXP repo = GET_SLOT(VECTOR_ELT(fetch_heads, 0), Rf_install("repo"));
        repository = git2r_repository_open(repo);
        if (!repository)
            git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);
    }

    err = git2r_merge_heads_from_fetch_heads(
        &merge_heads,
        repository,
        fetch_heads,
        n);
    if (err)
        goto cleanup;

    PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_merge_result")));
    err = git2r_merge(
        result,
        repository,
        (const git_annotated_commit **)merge_heads,
        n,
        GIT_MERGE_PREFERENCE_NONE,
        "pull",
        who,
        1); /* Commit on success */
    if (err)
        goto cleanup;

cleanup:
    if (who)
        git_signature_free(who);

    if (merge_heads)
        git2r_merge_heads_free(merge_heads, n);

    if (repository)
        git_repository_free(repository);

    if (!isNull(result))
        UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return result;
}
