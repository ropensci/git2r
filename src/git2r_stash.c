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
#include "git2r_repository.h"
#include "git2r_S3.h"
#include "git2r_signature.h"
#include "git2r_stash.h"

/**
 * Data structure to hold information when iterating over stash
 * objects.
 */
typedef struct {
    size_t n;
    SEXP list;
    SEXP repo;
    git_repository *repository;
} git2r_stash_list_cb_data;

/**
 * Apply a single stashed state from the stash list.
 *
 * @param repo S3 class git_repository that contains the stash
 * @param index The index to the stash. 0 is the most recent stash.
 * @return R_NilValue
 */
SEXP attribute_hidden
git2r_stash_apply(
    SEXP repo,
    SEXP index)
{
    int error;
    git_repository *repository = NULL;

    if (git2r_arg_check_integer_gte_zero(index))
        git2r_error(__func__, NULL, "'index'", git2r_err_integer_gte_zero_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git_stash_apply(repository, INTEGER(index)[0], NULL);
    if (error == GIT_ENOTFOUND)
        error = 0;
    git_repository_free(repository);
    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return R_NilValue;
}

/**
 * Remove a stash from the stash list
 *
 * @param repo S3 class git_repository that contains the stash
 * @param index The index to the stash. 0 is the most recent stash.
 * @return R_NilValue
 */
SEXP attribute_hidden
git2r_stash_drop(
    SEXP repo,
    SEXP index)
{
    int error;
    git_repository *repository = NULL;

    if (git2r_arg_check_integer_gte_zero(index))
        git2r_error(__func__, NULL, "'index'", git2r_err_integer_gte_zero_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git_stash_drop(repository, INTEGER(index)[0]);
    git_repository_free(repository);
    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return R_NilValue;
}

/**
 * Apply a single stashed state from the stash list and remove it from
 * the list if successful.
 *
 * @param repo S3 class git_repository that contains the stash
 * @param index The index to the stash. 0 is the most recent stash.
 * @return R_NilValue
 */
SEXP attribute_hidden
git2r_stash_pop(
    SEXP repo,
    SEXP index)
{
    int error;
    git_repository *repository = NULL;

    if (git2r_arg_check_integer_gte_zero(index))
        git2r_error(__func__, NULL, "'index'", git2r_err_integer_gte_zero_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git_stash_pop(repository, INTEGER(index)[0], NULL);
    if (error == GIT_ENOTFOUND)
        error = 0;
    git_repository_free(repository);
    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return R_NilValue;
}

/**
 * Init slots in S3 class git_stash
 *
 * @param source The commit oid of the stashed state.
 * @param repository The repository
 * @param repo S3 class git_repository that contains the stash
 * @param dest S3 class git_stash to initialize
 * @return int 0 on success, or an error code.
 */
static int
git2r_stash_init(
    const git_oid *source,
    git_repository *repository,
    SEXP repo,
    SEXP dest)
{
    int error;
    git_commit *commit = NULL;

    error = git_commit_lookup(&commit, repository, source);
    if (error)
        return error;
    git2r_commit_init(commit, repo, dest);
    git_commit_free(commit);

    return 0;
}

/**
 * Callback when iterating over stashes
 *
 * @param index The position within the stash list. 0 points to the
 * most recent stashed state.
 * @param message The stash message.
 * @param stash_id The commit oid of the stashed state.
 * @param payload Pointer to a git2r_stash_list_cb_data data structure.
 * @return 0 if OK, else error code
 */
static int
git2r_stash_list_cb(
    size_t index,
    const char* message,
    const git_oid *stash_id,
    void *payload)
{
    int error = 0, nprotect = 0;
    SEXP stash, class;
    git2r_stash_list_cb_data *cb_data = (git2r_stash_list_cb_data*)payload;

    GIT2R_UNUSED(index);
    GIT2R_UNUSED(message);

    /* Check if we have a list to populate */
    if (!Rf_isNull(cb_data->list)) {
        PROTECT(class = Rf_allocVector(STRSXP, 2));
        nprotect++;
        SET_STRING_ELT(class, 0, Rf_mkChar("git_stash"));
        SET_STRING_ELT(class, 1, Rf_mkChar("git_commit"));

        PROTECT(stash = Rf_mkNamed(VECSXP, git2r_S3_items__git_commit));
        nprotect++;
        Rf_setAttrib(stash, R_ClassSymbol, class);

        error = git2r_stash_init(
            stash_id,
            cb_data->repository,
            cb_data->repo,
            stash);
        if (error)
            goto cleanup;

        SET_VECTOR_ELT(cb_data->list, cb_data->n, stash);
    }

    cb_data->n += 1;

cleanup:
    if (nprotect)
        UNPROTECT(nprotect);

    return error;
}

/**
 * List stashes in a repository
 *
 * @param repo S3 class git_repository
 * @return VECXSP with S3 objects of class git_stash
 */
SEXP attribute_hidden
git2r_stash_list(
    SEXP repo)
{
    SEXP list = R_NilValue;
    int error, nprotect = 0;
    git2r_stash_list_cb_data cb_data = {0, R_NilValue, R_NilValue, NULL};
    git_repository *repository = NULL;

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    /* Count number of stashes before creating the list */
    error = git_stash_foreach(repository, &git2r_stash_list_cb, &cb_data);
    if (error)
        goto cleanup;

    PROTECT(list = Rf_allocVector(VECSXP, cb_data.n));
    nprotect++;
    cb_data.n = 0;
    cb_data.list = list;
    cb_data.repo = repo;
    cb_data.repository = repository;
    error = git_stash_foreach(repository, &git2r_stash_list_cb, &cb_data);

cleanup:
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return list;
}

/**
 * Stash
 *
 * @param repo The repository
 * @param message Optional description
 * @param index All changes already added to the index are left
 *        intact in the working directory. Default is FALSE
 * @param untracked All untracked files are also stashed and then
 *        cleaned up from the working directory. Default is FALSE
 * @param ignored All ignored files are also stashed and then cleaned
 *        up from the working directory. Default is FALSE
 * @param stasher Signature with stasher and time of stash
 * @return S3 class git_stash
 */
SEXP attribute_hidden
git2r_stash_save(
    SEXP repo,
    SEXP message,
    SEXP index,
    SEXP untracked,
    SEXP ignored,
    SEXP stasher)
{
    int error, nprotect = 0;
    SEXP result = R_NilValue, class;
    git_oid oid;
    git_stash_flags flags = GIT_STASH_DEFAULT;
    git_commit *commit = NULL;
    git_repository *repository = NULL;
    git_signature *c_stasher = NULL;

    if (git2r_arg_check_logical(index))
        git2r_error(__func__, NULL, "'index'", git2r_err_logical_arg);
    if (git2r_arg_check_logical(untracked))
        git2r_error(__func__, NULL, "'untracked'", git2r_err_logical_arg);
    if (git2r_arg_check_logical(ignored))
        git2r_error(__func__, NULL, "'ignored'", git2r_err_logical_arg);
    if (git2r_arg_check_signature(stasher))
        git2r_error(__func__, NULL, "'stasher'", git2r_err_signature_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    if (LOGICAL(index)[0])
        flags |= GIT_STASH_KEEP_INDEX;
    if (LOGICAL(untracked)[0])
        flags |= GIT_STASH_INCLUDE_UNTRACKED;
    if (LOGICAL(ignored)[0])
        flags |= GIT_STASH_INCLUDE_IGNORED;

    error = git2r_signature_from_arg(&c_stasher, stasher);
    if (error)
        goto cleanup;

    error = git_stash_save(
        &oid,
        repository,
        c_stasher,
        CHAR(STRING_ELT(message, 0)),
        flags);
    if (error) {
        if (GIT_ENOTFOUND == error)
            error = GIT_OK;
        goto cleanup;
    }

    PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_commit));
    nprotect++;
    Rf_setAttrib(result, R_ClassSymbol, class = Rf_allocVector(STRSXP, 2));
    SET_STRING_ELT(class, 0, Rf_mkChar("git_stash"));
    SET_STRING_ELT(class, 1, Rf_mkChar("git_commit"));
    error = git2r_stash_init(&oid, repository, repo, result);

cleanup:
    git_commit_free(commit);
    git_signature_free(c_stasher);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}
