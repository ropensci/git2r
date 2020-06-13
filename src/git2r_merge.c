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
#include "git2r_commit.h"
#include "git2r_deprecated.h"
#include "git2r_error.h"
#include "git2r_merge.h"
#include "git2r_repository.h"
#include "git2r_S3.h"
#include "git2r_signature.h"

int
git2r_commit_create(
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
SEXP attribute_hidden
git2r_merge_base(
    SEXP one,
    SEXP two)
{
    int error, nprotect = 0;
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

    repo_one = git2r_get_list_element(one, "repo");
    repo_two = git2r_get_list_element(two, "repo");
    if (git2r_arg_check_same_repo(repo_one, repo_two))
        git2r_error(__func__, NULL, "'one' and 'two' not from same repository", NULL);

    repository = git2r_repository_open(repo_one);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    sha = git2r_get_list_element(one, "sha");
    error = git_oid_fromstr(&oid_one, CHAR(STRING_ELT(sha, 0)));
    if (error)
        goto cleanup;

    sha = git2r_get_list_element(two, "sha");
    error = git_oid_fromstr(&oid_two, CHAR(STRING_ELT(sha, 0)));
    if (error)
        goto cleanup;

    error = git_merge_base(&oid, repository, &oid_one, &oid_two);
    if (error) {
        if (GIT_ENOTFOUND == error)
            error = GIT_OK;
        goto cleanup;
    }

    error = git_commit_lookup(&commit, repository, &oid);
    if (error)
        goto cleanup;

    PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_commit));
    nprotect++;
    Rf_setAttrib(result, R_ClassSymbol,
                 Rf_mkString(git2r_S3_class__git_commit));
    git2r_commit_init(commit, repo_one, result);

cleanup:
    git_commit_free(commit);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Perform a fast-forward merge
 *
 * @param merge_result S3 class git_merge_result
 * @param merge_head The merge head to fast-forward merge
 * @param repository The repository
 * @param log_message First part of the one line long message in the reflog
 * @return 0 on success, or error code
 */
static int
git2r_fast_forward_merge(
    SEXP merge_result,
    const git_annotated_commit *merge_head,
    git_repository *repository,
    const char *log_message)
{
    int error;
    const git_oid *oid;
    char *buf = NULL;
    size_t buf_len;
    git_commit *commit = NULL;
    git_tree *tree = NULL;
    git_reference *reference = NULL;
    git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;

    oid = git_annotated_commit_id(merge_head);
    error = git_commit_lookup(&commit, repository, oid);
    if (error)
        goto cleanup;

    error = git_commit_tree(&tree, commit);
    if (error)
        goto cleanup;

    opts.checkout_strategy = GIT_CHECKOUT_SAFE;
    error = git_checkout_tree(repository, (git_object*)tree, &opts);
    if (error)
        goto cleanup;

    error = git_repository_head(&reference, repository);
    if (error) {
        if (GIT_ENOTFOUND != error)
            goto cleanup;
    }

    buf_len = strlen(log_message) + sizeof(": Fast-forward");
    buf = malloc(buf_len);
    if (!buf) {
        GIT2R_ERROR_SET_OOM();
        error = GIT2R_ERROR_NOMEMORY;
        goto cleanup;
    }
    error = snprintf(buf, buf_len, "%s: Fast-forward", log_message);
    if (error < 0 || (size_t)error >= buf_len) {
        GIT2R_ERROR_SET_STR(GIT2R_ERROR_OS, "Failed to snprintf log message.");
        error = GIT2R_ERROR_OS;
        goto cleanup;
    }

    if (GIT_ENOTFOUND == error) {
        error = git_reference_create(
            &reference,
            repository,
            "HEAD",
            git_commit_id(commit),
            0, /* force */
            buf);
    } else {
        git_reference *target_ref = NULL;

        error = git_reference_set_target(
            &target_ref,
            reference,
            git_commit_id(commit),
            buf);

        if (target_ref)
            git_reference_free(target_ref);
    }

    SET_VECTOR_ELT(
        merge_result,
        git2r_S3_item__git_merge_result__fast_forward,
        Rf_ScalarLogical(1));

    SET_VECTOR_ELT(
        merge_result,
        git2r_S3_item__git_merge_result__conflicts,
        Rf_ScalarLogical(0));

    SET_VECTOR_ELT(
        merge_result,
        git2r_S3_item__git_merge_result__sha,
        Rf_ScalarString(NA_STRING));

cleanup:
    if (buf)
        free(buf);
    git_commit_free(commit);
    git_reference_free(reference);
    git_tree_free(tree);

    return error;
}

/**
 * Perform a normal merge
 *
 * @param merge_result S3 class git_merge_result
 * @param merge_heads The merge heads to merge
 * @param n The number of merge heads
 * @param repository The repository
 * @param message The commit message of the merge
 * @param merger Who is performing the merge
 * @param commit_on_success Commit merge commit, if one was created
 * @param merge_opts Merge options
 * @return 0 on success, or error code
 */
static int
git2r_normal_merge(
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
    int error;
    git_commit *commit = NULL;
    git_index *index = NULL;

    SET_VECTOR_ELT(
        merge_result,
        git2r_S3_item__git_merge_result__fast_forward,
        Rf_ScalarLogical(0));

    error = git_merge(
        repository,
        merge_heads,
        n,
        merge_opts,
        checkout_opts);
    if (error) {
        if (error == GIT_EMERGECONFLICT) {
            SET_VECTOR_ELT(
                merge_result,
                git2r_S3_item__git_merge_result__conflicts,
                Rf_ScalarLogical(1));

            SET_VECTOR_ELT(
                merge_result,
                git2r_S3_item__git_merge_result__sha,
                Rf_ScalarString(NA_STRING));

            error = 0;
        }
        goto cleanup;
    }

    error = git_repository_index(&index, repository);
    if (error)
        goto cleanup;

    if (git_index_has_conflicts(index)) {
        SET_VECTOR_ELT(
            merge_result,
            git2r_S3_item__git_merge_result__conflicts,
            Rf_ScalarLogical(1));

        SET_VECTOR_ELT(
            merge_result,
            git2r_S3_item__git_merge_result__sha,
            Rf_ScalarString(NA_STRING));
    } else {
        SET_VECTOR_ELT(
            merge_result,
            git2r_S3_item__git_merge_result__conflicts,
            Rf_ScalarLogical(0));

        if (commit_on_success) {
            char sha[GIT_OID_HEXSZ + 1];
            git_oid oid;

            error = git2r_commit_create(
                &oid,
                repository,
                index,
                message,
                merger,
                merger);
            if (error)
                goto cleanup;

            git_oid_fmt(sha, &oid);
            sha[GIT_OID_HEXSZ] = '\0';
            SET_VECTOR_ELT(
                merge_result,
                git2r_S3_item__git_merge_result__sha,
                Rf_mkString(sha));
        }
    }

cleanup:
    git_commit_free(commit);
    git_index_free(index);

    return error;
}

/**
 * @param merge_result S3 class git_merge_result
 * @repository The repository
 * @param merge_head The merge head to merge
 * @param n The number of merge heads
 * @param preference The merge preference option (None [0], No
 * Fast-Forward [1] or Only Fast-Forward [2])
 * @param name The name of the merge in the reflog
 * @param merger Who is performing the merge
 * @param commit_on_success Commit merge commit, if one was created
 * during a normal merge
 * @param fail If a conflict occurs, exit immediately instead of attempting to
 * continue resolving conflicts.
 * @return 0 on success, or error code
 */
static int
git2r_merge(
    SEXP merge_result,
    git_repository *repository,
    const git_annotated_commit **merge_heads,
    size_t n,
    git_merge_preference_t preference,
    const char *name,
    git_signature *merger,
    int commit_on_success,
    int fail)
{
    int error;
    git_merge_analysis_t merge_analysis;
    git_merge_preference_t merge_preference;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
    git_merge_options merge_opts = GIT_MERGE_OPTIONS_INIT;

    merge_opts.rename_threshold = 50;
    merge_opts.target_limit = 200;
    if (fail)
        merge_opts.flags |= GIT_MERGE_FAIL_ON_CONFLICT;

    checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;

    error = git_merge_analysis(
        &merge_analysis,
        &merge_preference,
        repository,
        merge_heads,
        n);
    if (error)
        return error;

    if (merge_analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) {
        SET_VECTOR_ELT(
            merge_result,
            git2r_S3_item__git_merge_result__up_to_date,
            Rf_ScalarLogical(1));
        SET_VECTOR_ELT(
            merge_result,
            git2r_S3_item__git_merge_result__fast_forward,
            Rf_ScalarLogical(0));
        SET_VECTOR_ELT(
            merge_result,
            git2r_S3_item__git_merge_result__conflicts,
            Rf_ScalarLogical(0));
        SET_VECTOR_ELT(
            merge_result,
            git2r_S3_item__git_merge_result__sha,
            Rf_ScalarString(NA_STRING));
        return 0;
    } else {
        SET_VECTOR_ELT(
            merge_result,
            git2r_S3_item__git_merge_result__up_to_date,
            Rf_ScalarLogical(0));
    }

    if (GIT_MERGE_PREFERENCE_NONE == preference)
        preference = merge_preference;

    switch (preference) {
    case GIT_MERGE_PREFERENCE_NONE:
        if (merge_analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) {
            if (1 != n) {
                GIT2R_ERROR_SET_STR(
                    GIT2R_ERROR_NONE,
                    "Unable to perform Fast-Forward merge "
                    "with mith multiple merge heads.");
                return GIT_ERROR;
            }

            error = git2r_fast_forward_merge(
                merge_result,
                merge_heads[0],
                repository,
                name);
        } else if (merge_analysis & GIT_MERGE_ANALYSIS_NORMAL) {
            error = git2r_normal_merge(
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
            error = git2r_normal_merge(
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
                GIT2R_ERROR_SET_STR(
                    GIT2R_ERROR_NONE,
                    "Unable to perform Fast-Forward merge "
                    "with mith multiple merge heads.");
                return GIT_ERROR;
            }

            error = git2r_fast_forward_merge(
                merge_result,
                merge_heads[0],
                repository,
                name);
        } else {
            GIT2R_ERROR_SET_STR(GIT2R_ERROR_NONE, "Unable to perform Fast-Forward merge.");
            return GIT_ERROR;
        }
        break;
    default:
        GIT2R_ERROR_SET_STR(GIT2R_ERROR_NONE, "Unknown merge option");
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
static void
git2r_merge_heads_free(
    git_annotated_commit **merge_heads,
    size_t n)
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
 * @param branch S3 class git_branch to merge into HEAD.
 * @param merger Who is performing the merge
 * @param commit_on_success Commit merge commit, if one was created
 * during a normal merge
 * @param fail If a conflict occurs, exit immediately instead of attempting to
 * continue resolving conflicts.
 * @return S3 class git_merge_result
 */
SEXP attribute_hidden
git2r_merge_branch(
    SEXP branch,
    SEXP merger,
    SEXP commit_on_success,
    SEXP fail)
{
    int error, nprotect = 0;
    SEXP result = R_NilValue;
    const char *name;
    char *buf = NULL;
    size_t buf_len;
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

    error = git2r_signature_from_arg(&who, merger);
    if (error)
        goto cleanup;

    repository = git2r_repository_open(git2r_get_list_element(branch, "repo"));
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    name = CHAR(STRING_ELT(git2r_get_list_element(branch, "name"), 0));
    type = INTEGER(git2r_get_list_element(branch, "type"))[0];
    error = git_branch_lookup(&reference, repository, name, type);
    if (error)
        goto cleanup;

    merge_heads = calloc(1, sizeof(git_annotated_commit*));
    if (NULL == merge_heads) {
        GIT2R_ERROR_SET_STR(GIT2R_ERROR_NONE, git2r_err_alloc_memory_buffer);
        goto cleanup;
    }

    error = git_annotated_commit_from_ref(
        &(merge_heads[0]),
        repository,
        reference);
    if (error)
        goto cleanup;

    buf_len = strlen(name) + sizeof("merge ");
    buf = malloc(buf_len);
    if (!buf) {
        GIT2R_ERROR_SET_OOM();
        error = GIT2R_ERROR_NOMEMORY;
        goto cleanup;
    }
    error = snprintf(buf, buf_len, "merge %s", name);
    if (error < 0 || (size_t)error >= buf_len) {
        GIT2R_ERROR_SET_STR(GIT2R_ERROR_OS, "Failed to snprintf log message.");
        error = GIT2R_ERROR_OS;
        goto cleanup;
    }

    PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_merge_result));
    nprotect++;
    Rf_setAttrib(result, R_ClassSymbol,
                 Rf_mkString(git2r_S3_class__git_merge_result));
    error = git2r_merge(
        result,
        repository,
        (const git_annotated_commit **)merge_heads,
        1,
        GIT_MERGE_PREFERENCE_NONE,
        buf,
        who,
        LOGICAL(commit_on_success)[0],
        LOGICAL(fail)[0]);

cleanup:
    if (buf)
        free(buf);
    git_signature_free(who);
    git2r_merge_heads_free(merge_heads, 1);
    git_reference_free(reference);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Create and populate a vector of git_annotated_commit objects from
 * the given fetch head data.
 *
 * @param out Pointer the vector of git_annotated_commit objects.
 * @param repository The repository.
 * @param fetch_heads List of S3 class git_fetch_head objects.
 * @param n Length of fetch_heads list.
 * @return 0 on success, or error code
 */
static int
git2r_merge_heads_from_fetch_heads(
    git_annotated_commit ***merge_heads,
    git_repository *repository,
    SEXP fetch_heads,
    size_t n)
{
    int error = GIT_OK;
    size_t i;

    *merge_heads = calloc(n, sizeof(git_annotated_commit*));
    if (!(*merge_heads)) {
        GIT2R_ERROR_SET_STR(GIT2R_ERROR_NONE, git2r_err_alloc_memory_buffer);
        return GIT_ERROR;
    }

    for (i = 0; i < n; i++) {
        git_oid oid;
        SEXP fh = VECTOR_ELT(fetch_heads, i);

        error = git_oid_fromstr(
            &oid,
            CHAR(STRING_ELT(git2r_get_list_element(fh, "sha"), 0)));
        if (error)
            goto cleanup;

        error = git_annotated_commit_from_fetchhead(
            &((*merge_heads)[i]),
            repository,
            CHAR(STRING_ELT(git2r_get_list_element(fh, "ref_name"), 0)),
            CHAR(STRING_ELT(git2r_get_list_element(fh, "remote_url"), 0)),
            &oid);
        if (error)
            goto cleanup;
    }

cleanup:
    if (error) {
        if (*merge_heads)
            git2r_merge_heads_free(*merge_heads, n);
        *merge_heads = NULL;
    }

    return error;
}

/**
 * Merge the given fetch head data into HEAD
 *
 * @param fetch_heads List of S3 class git_fetch_head objects.
 * @param merger Who made the merge, if the merge is non-fast forward
 * merge that creates a merge commit.
 * @return List of git_annotated_commit objects.
 */
SEXP attribute_hidden
git2r_merge_fetch_heads(
    SEXP fetch_heads,
    SEXP merger)
{
    int error, nprotect = 0;
    size_t n = 0;
    SEXP result = R_NilValue;
    git_annotated_commit **merge_heads = NULL;
    git_repository *repository = NULL;
    git_signature *who = NULL;

    if (git2r_arg_check_fetch_heads(fetch_heads))
        git2r_error(__func__, NULL, "'fetch_heads'", git2r_err_fetch_heads_arg);
    if (git2r_arg_check_signature(merger))
        git2r_error(__func__, NULL, "'merger'", git2r_err_signature_arg);

    error = git2r_signature_from_arg(&who, merger);
    if (error)
        goto cleanup;

    n = LENGTH(fetch_heads);
    if (n) {
        SEXP repo = git2r_get_list_element(VECTOR_ELT(fetch_heads, 0), "repo");
        repository = git2r_repository_open(repo);
        if (!repository)
            git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);
    }

    error = git2r_merge_heads_from_fetch_heads(
        &merge_heads,
        repository,
        fetch_heads,
        n);
    if (error)
        goto cleanup;

    PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_merge_result));
    nprotect++;
    Rf_setAttrib(result, R_ClassSymbol,
                 Rf_mkString(git2r_S3_class__git_merge_result));

    error = git2r_merge(
        result,
        repository,
        (const git_annotated_commit **)merge_heads,
        n,
        GIT_MERGE_PREFERENCE_NONE,
        "pull",
        who,
        1,  /* Commit on success */
        0); /* Don't fail on conflicts */

cleanup:
    git_signature_free(who);
    git2r_merge_heads_free(merge_heads, n);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}
