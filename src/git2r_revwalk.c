/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2021 The git2r contributors
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
#include "git2r_oid.h"
#include "git2r_repository.h"
#include "git2r_S3.h"

/**
 * Count number of revisions.
 *
 * @param walker The walker to pop the commit from.
 * @param max_n n The upper limit of the number of commits to
 * output. Use max_n < 0 for unlimited number of commits.
 * @return The number of revisions
 */
static int
git2r_revwalk_count(
    git_revwalk *walker,
    int max_n)
{
    int n = 0;
    git_oid oid;

    while (!git_revwalk_next(&oid, walker)) {
        if (max_n < 0 || n < max_n)
            n++;
        else
            break;
    }

    return n;
}

/* Helper to find how many files in a commit changed from its nth
 * parent. */
static int
git2r_match_with_parent(
    int *out,
    git_commit *commit,
    unsigned int i,
    git_diff_options *opts)
{
    int error;
    git_commit *parent = NULL;
    git_tree *a = NULL, *b = NULL;
    git_diff *diff = NULL;

    if ((error = git_commit_parent(&parent, commit, i)) < 0)
        goto cleanup;
    if ((error = git_commit_tree(&a, parent)) < 0)
        goto cleanup;
    if ((error = git_commit_tree(&b, commit)) < 0)
        goto cleanup;
    if ((error = git_diff_tree_to_tree(&diff, git_commit_owner(commit), a, b, opts)) < 0)
        goto cleanup;

    *out = (git_diff_num_deltas(diff) > 0);

cleanup:
    git_diff_free(diff);
    git_tree_free(a);
    git_tree_free(b);
    git_commit_free(parent);

    return error;
}

/**
 * List revisions
 *
 * @param repo S3 class git_repository
 * @param sha id of the commit to start from.
 * @param topological Sort the commits by topological order; Can be
 * combined with time.
 * @param time Sort the commits by commit time; can be combined with
 * topological.
 * @param reverse Sort the commits in reverse order
 * @param max_n n The upper limit of the number of commits to
 * output. Use max_n < 0 for unlimited number of commits.
 * @return list with S3 class git_commit objects
 */
SEXP attribute_hidden
git2r_revwalk_list(
    SEXP repo,
    SEXP sha,
    SEXP topological,
    SEXP time,
    SEXP reverse,
    SEXP max_n)
{
    int error = GIT_OK, nprotect = 0;
    SEXP result = R_NilValue;
    int i, n;
    unsigned int sort_mode = GIT_SORT_NONE;
    git_revwalk *walker = NULL;
    git_repository *repository = NULL;
    git_oid oid;

    if (git2r_arg_check_sha(sha))
        git2r_error(__func__, NULL, "'sha'", git2r_err_sha_arg);
    if (git2r_arg_check_logical(topological))
        git2r_error(__func__, NULL, "'topological'", git2r_err_logical_arg);
    if (git2r_arg_check_logical(time))
        git2r_error(__func__, NULL, "'time'", git2r_err_logical_arg);
    if (git2r_arg_check_logical(reverse))
        git2r_error(__func__, NULL, "'reverse'", git2r_err_logical_arg);
    if (git2r_arg_check_integer(max_n))
        git2r_error(__func__, NULL, "'max_n'", git2r_err_integer_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    if (git_repository_is_empty(repository)) {
        /* No commits, create empty list */
        PROTECT(result = Rf_allocVector(VECSXP, 0));
        nprotect++;
        goto cleanup;
    }

    if (LOGICAL(topological)[0])
        sort_mode |= GIT_SORT_TOPOLOGICAL;
    if (LOGICAL(time)[0])
        sort_mode |= GIT_SORT_TIME;
    if (LOGICAL(reverse)[0])
        sort_mode |= GIT_SORT_REVERSE;

    error = git_revwalk_new(&walker, repository);
    if (error)
        goto cleanup;

    git2r_oid_from_sha_sexp(sha, &oid);
    error = git_revwalk_push(walker, &oid);
    if (error)
        goto cleanup;
    git_revwalk_sorting(walker, sort_mode);

    /* Count number of revisions before creating the list */
    n = git2r_revwalk_count(walker, INTEGER(max_n)[0]);

    /* Create list to store result */
    PROTECT(result = Rf_allocVector(VECSXP, n));
    nprotect++;

    git_revwalk_reset(walker);
    error = git_revwalk_push(walker, &oid);
    if (error)
        goto cleanup;
    git_revwalk_sorting(walker, sort_mode);

    for (i = 0; i < n; i++) {
        git_commit *commit;
        SEXP item;
        git_oid oid;

        error = git_revwalk_next(&oid, walker);
        if (error) {
            if (GIT_ITEROVER == error)
                error = GIT_OK;
            goto cleanup;
        }

        error = git_commit_lookup(&commit, repository, &oid);
        if (error)
            goto cleanup;

        SET_VECTOR_ELT(
            result,
            i,
            item = Rf_mkNamed(VECSXP, git2r_S3_items__git_commit));
        Rf_setAttrib(item, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_commit));
        git2r_commit_init(commit, repo, item);
        git_commit_free(commit);
    }

cleanup:
    git_revwalk_free(walker);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * List revisions modifying a particular path.
 *
 * @param repo S3 class git_repository
 * @param sha id of the commit to start from.
 * @param topological Sort the commits by topological order; Can be
 * combined with time.
 * @param time Sort the commits by commit time; can be combined with
 * topological.
 * @param reverse Sort the commits in reverse order
 * @param max_n n The upper limit of the number of commits to
 * output. Use max_n < 0 for unlimited number of commits.
 * @param path Only commits modifying this path are selected
 * @return list with S3 class git_commit objects
 */
SEXP attribute_hidden
git2r_revwalk_list2 (
    SEXP repo,
    SEXP sha,
    SEXP topological,
    SEXP time,
    SEXP reverse,
    SEXP max_n,
    SEXP path)
{
    int i = 0;
    int error = GIT_OK;
    int nprotect = 0;
    SEXP result = R_NilValue;
    int n;
    unsigned int sort_mode = GIT_SORT_NONE;
    git_revwalk *walker = NULL;
    git_repository *repository = NULL;
    git_oid oid;
    git_diff_options diffopts = GIT_DIFF_OPTIONS_INIT;
    git_pathspec *ps = NULL;

    if (git2r_arg_check_sha(sha))
        git2r_error(__func__, NULL, "'sha'", git2r_err_sha_arg);
    if (git2r_arg_check_logical(topological))
        git2r_error(__func__, NULL, "'topological'", git2r_err_logical_arg);
    if (git2r_arg_check_logical(time))
        git2r_error(__func__, NULL, "'time'", git2r_err_logical_arg);
    if (git2r_arg_check_logical(reverse))
        git2r_error(__func__, NULL, "'reverse'", git2r_err_logical_arg);
    if (git2r_arg_check_integer(max_n))
        git2r_error(__func__, NULL, "'max_n'", git2r_err_integer_arg);
    if (git2r_arg_check_string(path))
        git2r_error(__func__, NULL, "'path'", git2r_err_string_arg);

    /* Set up git pathspec. */
    error = git2r_copy_string_vec(&(diffopts.pathspec), path);
    if (error || !diffopts.pathspec.count)
        goto cleanup;
    error = git_pathspec_new(&ps, &diffopts.pathspec);
    if (error)
        goto cleanup;

    /* Open the repository. */
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    /* If there are no commits, create an empty list. */
    if (git_repository_is_empty(repository)) {
        PROTECT(result = Rf_allocVector(VECSXP, 0));
        nprotect++;
        goto cleanup;
    }

    if (LOGICAL(topological)[0])
        sort_mode |= GIT_SORT_TOPOLOGICAL;
    if (LOGICAL(time)[0])
        sort_mode |= GIT_SORT_TIME;
    if (LOGICAL(reverse)[0])
        sort_mode |= GIT_SORT_REVERSE;

    /* Create a new "revwalker". */
    error = git_revwalk_new(&walker, repository);
    if (error)
        goto cleanup;

    git2r_oid_from_sha_sexp(sha, &oid);
    error = git_revwalk_push(walker, &oid);
    if (error)
        goto cleanup;
    git_revwalk_sorting(walker, sort_mode);
    error = git_revwalk_push_head(walker);
    if (error)
        goto cleanup;

    n = Rf_asInteger(max_n);
    if (n < 0) {
        /* Count number of revisions before creating the list. */
        n = git2r_revwalk_count(walker, n);
    }

    /* Restart the revwalker. */
    git_revwalk_reset(walker);
    git_revwalk_sorting(walker, sort_mode);
    error = git_revwalk_push(walker, &oid);
    if (error)
        goto cleanup;

    /* Create the list to store the result. */
    PROTECT(result = Rf_allocVector(VECSXP, n));
    nprotect++;

    while (i < n) {
        git_commit *commit;
        SEXP item;
        git_oid oid;
	unsigned int parents, unmatched;
        int match;

        error = git_revwalk_next(&oid, walker);
        if (error) {
            if (GIT_ITEROVER == error)
                error = GIT_OK;
            goto cleanup;
        }

        error = git_commit_lookup(&commit, repository, &oid);
        if (error)
            goto cleanup;

        /* Check whether it is a "touching" commit---that is, a commit
	   that has modified the selected path. */
        parents = git_commit_parentcount(commit);
        unmatched = parents;
        if (parents == 0) {
	    git_tree *tree;
	    if ((error = git_commit_tree(&tree, commit)) < 0) {
                git_commit_free(commit);
                goto cleanup;
            }
            error = git_pathspec_match_tree(
                NULL, tree, GIT_PATHSPEC_NO_MATCH_ERROR, ps);
	    git_tree_free(tree);
	    if (error == GIT_ENOTFOUND) {
                error = 0;
	        unmatched = 1;
            } else if (error < 0) {
                git_commit_free(commit);
                goto cleanup;
            }
	} else if (parents == 1) {
            if ((error = git2r_match_with_parent(&match, commit, 0, &diffopts)) < 0) {
                git_commit_free(commit);
                goto cleanup;
            }
            unmatched = match ? 0 : 1;
	} else {
            unsigned int j;

            for (j = 0; j < parents; j++) {
                if ((error = git2r_match_with_parent(&match, commit, j, &diffopts)) < 0) {
                    git_commit_free(commit);
                    goto cleanup;
                }
                if (match && unmatched)
                    unmatched--;
            }
	}

        if (unmatched > 0) {
            git_commit_free(commit);
            continue;
        }

        SET_VECTOR_ELT(
            result,
            i,
            item = Rf_mkNamed(VECSXP, git2r_S3_items__git_commit));
        Rf_setAttrib(item, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_commit));
        git2r_commit_init(commit, repo, item);
        git_commit_free(commit);
        i++;
    }

cleanup:
    free(diffopts.pathspec.strings);
    git_revwalk_free(walker);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Get list with contributions.
 *
 * @param repo S3 class git_repository
 * @param topological Sort the commits by topological order; Can be
 * combined with time.
 * @param time Sort the commits by commit time; can be combined with
 * topological.
 * @param reverse Sort the commits in reverse order
 * @return list with S3 class git_commit objects
 */
SEXP attribute_hidden
git2r_revwalk_contributions(
    SEXP repo,
    SEXP topological,
    SEXP time,
    SEXP reverse)
{
    int error = GIT_OK, nprotect = 0;
    SEXP result = R_NilValue;
    SEXP names = R_NilValue;
    SEXP when = R_NilValue;
    SEXP author = R_NilValue;
    SEXP email = R_NilValue;
    size_t i, n = 0;
    unsigned int sort_mode = GIT_SORT_NONE;
    git_revwalk *walker = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_logical(topological))
        git2r_error(__func__, NULL, "'topological'", git2r_err_logical_arg);
    if (git2r_arg_check_logical(time))
        git2r_error(__func__, NULL, "'time'", git2r_err_logical_arg);
    if (git2r_arg_check_logical(reverse))
        git2r_error(__func__, NULL, "'reverse'", git2r_err_logical_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    if (git_repository_is_empty(repository))
        goto cleanup;

    if (LOGICAL(topological)[0])
        sort_mode |= GIT_SORT_TOPOLOGICAL;
    if (LOGICAL(time)[0])
        sort_mode |= GIT_SORT_TIME;
    if (LOGICAL(reverse)[0])
        sort_mode |= GIT_SORT_REVERSE;

    error = git_revwalk_new(&walker, repository);
    if (error)
        goto cleanup;

    error = git_revwalk_push_head(walker);
    if (error)
        goto cleanup;
    git_revwalk_sorting(walker, sort_mode);

    /* Count number of revisions before creating the list */
    n = git2r_revwalk_count(walker, -1);

    /* Create vectors to store result */
    PROTECT(result = Rf_allocVector(VECSXP, 3));
    nprotect++;
    Rf_setAttrib(result, R_NamesSymbol, names = Rf_allocVector(STRSXP, 3));
    SET_VECTOR_ELT(result, 0, when = Rf_allocVector(REALSXP, n));
    SET_STRING_ELT(names, 0, Rf_mkChar("when"));
    SET_VECTOR_ELT(result, 1, author = Rf_allocVector(STRSXP, n));
    SET_STRING_ELT(names, 1, Rf_mkChar("author"));
    SET_VECTOR_ELT(result, 2, email = Rf_allocVector(STRSXP, n));
    SET_STRING_ELT(names, 2, Rf_mkChar("email"));

    git_revwalk_reset(walker);
    error = git_revwalk_push_head(walker);
    if (error)
        goto cleanup;
    git_revwalk_sorting(walker, sort_mode);

    for (i = 0; i < n; i++) {
        git_commit *commit;
        const git_signature *c_author;
        git_oid oid;

        error = git_revwalk_next(&oid, walker);
        if (error) {
            if (GIT_ITEROVER == error)
                error = GIT_OK;
            goto cleanup;
        }

        error = git_commit_lookup(&commit, repository, &oid);
        if (error)
            goto cleanup;

        c_author = git_commit_author(commit);
        REAL(when)[i] =
            (double)(c_author->when.time) +
            60.0 * (double)(c_author->when.offset);
        SET_STRING_ELT(author, i, Rf_mkChar(c_author->name));
        SET_STRING_ELT(author, i, Rf_mkChar(c_author->email));
        git_commit_free(commit);
    }

cleanup:
    git_revwalk_free(walker);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}
