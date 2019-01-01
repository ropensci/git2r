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

#include <git2.h>

#include "git2r_arg.h"
#include "git2r_commit.h"
#include "git2r_error.h"
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
static int git2r_revwalk_count(git_revwalk *walker, int max_n)
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

/**
 * List revisions
 *
 * @param repo S3 class git_repository
 * @param topological Sort the commits by topological order; Can be
 * combined with time.
 * @param time Sort the commits by commit time; can be combined with
 * topological.
 * @param reverse Sort the commits in reverse order
 * @param max_n n The upper limit of the number of commits to
 * output. Use max_n < 0 for unlimited number of commits.
 * @param pathname TO DO: Add description of "pathname" argument here.
 * @return list with S3 class git_commit objects
 */
SEXP git2r_revwalk_list(
    SEXP repo,
    SEXP topological,
    SEXP time,
    SEXP reverse,
    SEXP max_n,
    SEXP pathname)
{
    int error = GIT_OK, nprotect = 0;
    SEXP result = R_NilValue;
    int i, n;
    unsigned int sort_mode = GIT_SORT_NONE;
    git_revwalk *walker = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_logical(topological))
        git2r_error(__func__, NULL, "'topological'", git2r_err_logical_arg);
    if (git2r_arg_check_logical(time))
        git2r_error(__func__, NULL, "'time'", git2r_err_logical_arg);
    if (git2r_arg_check_logical(reverse))
        git2r_error(__func__, NULL, "'reverse'", git2r_err_logical_arg);
    if (git2r_arg_check_integer(max_n))
        git2r_error(__func__, NULL, "'max_n'", git2r_err_integer_arg);
    if (git2r_arg_check_string(pathname))
        git2r_error(__func__, NULL, "'pathname'", git2r_err_string_arg);
      
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

    error = git_revwalk_push_head(walker);
    if (error)
        goto cleanup;
    git_revwalk_sorting(walker, sort_mode);

    /* Count number of revisions before creating the list */
    n = git2r_revwalk_count(walker, INTEGER(max_n)[0]);

    /* Create list to store result */
    PROTECT(result = Rf_allocVector(VECSXP, n));
    nprotect++;

    git_revwalk_reset(walker);
    error = git_revwalk_push_head(walker);
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
        git2r_error(__func__, giterr_last(), NULL, NULL);

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
SEXP git2r_revwalk_contributions(
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
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return result;
}
