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

#include "git2r_arg.h"
#include "git2r_blob.h"
#include "git2r_branch.h"
#include "git2r_commit.h"
#include "git2r_error.h"
#include "git2r_repository.h"
#include "git2r_tag.h"
#include "git2r_tree.h"
#include "buffer.h"

/**
 * Get repo slot from S4 class git_repository
 *
 * @param repo S4 class git_repository
 * @return a git_repository pointer on success else NULL
 */
git_repository* git2r_repository_open(SEXP repo)
{
    SEXP class_name;
    SEXP path;
    git_repository *repository;

    if (R_NilValue == repo || S4SXP != TYPEOF(repo))
        return NULL;

    class_name = getAttrib(repo, R_ClassSymbol);
    if (0 != strcmp(CHAR(STRING_ELT(class_name, 0)), "git_repository"))
        return NULL;

    path = GET_SLOT(repo, Rf_install("path"));
    if (GIT_OK != git2r_arg_check_string(path))
        return NULL;

    if (git_repository_open(&repository, CHAR(STRING_ELT(path, 0))) < 0)
        return NULL;

    return repository;
}

/**
 * Data structure to hold information when iterating over FETCH_HEAD
 * entries.
 */
typedef struct {
    size_t n;
    SEXP list;
    SEXP repo;
} git2r_fetch_head_cb_data;

/**
 * Invoked 'callback' for each entry in the given FETCH_HEAD file.
 *
 * @param ref_name The name of the ref.
 * @param remote_url The url of the remote.
 * @param oid The id of the remote head that were updated during the
 * last fetch.
 * @param is_merge Is head for merge.
 * @return 0
 */
static int git2r_repository_fetchhead_foreach_cb(
    const char *ref_name,
    const char *remote_url,
    const git_oid *oid,
    unsigned int is_merge,
    void *payload)
{
    git2r_fetch_head_cb_data *cb_data = (git2r_fetch_head_cb_data*)payload;

    /* Check if we have a list to populate */
    if (R_NilValue != cb_data->list) {
        char sha[GIT_OID_HEXSZ + 1];
        SEXP fetch_head;

        SET_VECTOR_ELT(
            cb_data->list,
            cb_data->n,
            fetch_head = NEW_OBJECT(MAKE_CLASS("git_fetch_head")));

        SET_SLOT(fetch_head, Rf_install("ref_name"), mkString(ref_name));
        SET_SLOT(fetch_head, Rf_install("remote_url"), mkString(remote_url));
        git_oid_tostr(sha, sizeof(sha), oid);
        SET_SLOT(fetch_head, Rf_install("sha"), mkString(sha));
        SET_SLOT(fetch_head, Rf_install("is_merge"), ScalarLogical(is_merge));
        SET_SLOT(fetch_head, Rf_install("repo"), cb_data->repo);
    }

    cb_data->n += 1;

    return 0;
}

/**
 * Get entries in FETCH_HEAD file
 *
 * @param repo S4 class git_repository
 * @return list with the S4 class git_fetch_head entries. R_NilValue
 * if there is no FETCH_HEAD file.
 */
SEXP git2r_repository_fetch_heads(SEXP repo)
{
    int err;
    SEXP result = R_NilValue;
    git2r_fetch_head_cb_data cb_data = {0, R_NilValue, R_NilValue};
    git_repository *repository = NULL;

    repository= git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    /* Count number of fetch heads before creating the list */
    err = git_repository_fetchhead_foreach(
        repository,
        git2r_repository_fetchhead_foreach_cb,
        &cb_data);

    if (GIT_OK != err) {
        if (GIT_ENOTFOUND == err)
            err = GIT_OK;
        goto cleanup;
    }

    PROTECT(result = allocVector(VECSXP, cb_data.n));
    cb_data.n = 0;
    cb_data.list = result;
    cb_data.repo = repo;
    err = git_repository_fetchhead_foreach(
        repository,
        git2r_repository_fetchhead_foreach_cb,
        &cb_data);

cleanup:
    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (GIT_OK != err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return result;
}

/**
 * Get head of repository
 *
 * @param repo S4 class git_repository
 * @return R_NilValue if unborn branch or not found. S4 class
 * git_branch if not a detached head. S4 class git_commit if detached
 * head
 */
SEXP git2r_repository_head(SEXP repo)
{
    int err;
    SEXP result = R_NilValue;
    git_commit *commit = NULL;
    git_reference *reference = NULL;
    git_repository *repository = NULL;

    repository= git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git_repository_head(&reference, repository);
    if (GIT_OK != err) {
        if (GIT_EUNBORNBRANCH == err || GIT_ENOTFOUND == err)
            err = GIT_OK;
        goto cleanup;
    }

    if (git_reference_is_branch(reference)) {
        git_branch_t type = GIT_BRANCH_LOCAL;
        if (git_reference_is_remote(reference))
            type = GIT_BRANCH_REMOTE;
        PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_branch")));
        err = git2r_branch_init(reference, type, repo, result);
    } else {
        err = git_commit_lookup(
            &commit,
            repository,
            git_reference_target(reference));
        if (GIT_OK != err)
            goto cleanup;
        PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_commit")));
        git2r_commit_init(commit, repo, result);
    }

cleanup:
    if (commit)
        git_commit_free(commit);

    if (reference)
        git_reference_free(reference);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (GIT_OK != err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return result;
}

/**
 * Init a repository.
 *
 * @param path A path to where to init a git repository
 * @param bare If TRUE, a Git repository without a working directory
 * is created at the pointed path. If FALSE, provided path will be
 * considered as the working directory into which the .git directory
 * will be created.
 * @return R_NilValue
 */
SEXP git2r_repository_init(SEXP path, SEXP bare)
{
    int err;
    git_repository *repository = NULL;

    if (GIT_OK != git2r_arg_check_string(path))
        git2r_error(git2r_err_string_arg, __func__, "path");
    if (GIT_OK != git2r_arg_check_logical(bare))
        git2r_error(git2r_err_logical_arg, __func__, "bare");

    err = git_repository_init(&repository,
                              CHAR(STRING_ELT(path, 0)),
                              LOGICAL(bare)[0]);
    if (GIT_OK != err)
        git2r_error("Error in '%s': Unable to init repository", __func__, NULL);

    if (repository)
        git_repository_free(repository);

    return R_NilValue;
}

/**
 * Check if repository is bare.
 *
 * @param repo S4 class git_repository
 * @return TRUE if bare else FALSE
 */
SEXP git2r_repository_is_bare(SEXP repo)
{
    SEXP result;
    int is_bare;
    git_repository *repository;

    repository= git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    is_bare = git_repository_is_bare(repository);
    git_repository_free(repository);

    PROTECT(result = allocVector(LGLSXP, 1));
    if (1 == is_bare)
        LOGICAL(result)[0] = 1;
    else
        LOGICAL(result)[0] = 0;
    UNPROTECT(1);

    return result;
}

/**
 * Determine if the repository was a shallow clone.
 *
 * @param repo S4 class git_repository
 * @return TRUE if shallow else FALSE
 */
SEXP git2r_repository_is_shallow(SEXP repo)
{
    SEXP result;
    int is_shallow;
    git_repository *repository;

    repository= git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    is_shallow = git_repository_is_shallow(repository);
    git_repository_free(repository);
    if (is_shallow < 0)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    PROTECT(result = allocVector(LGLSXP, 1));
    if (1 == is_shallow)
        LOGICAL(result)[0] = 1;
    else
        LOGICAL(result)[0] = 0;
    UNPROTECT(1);

    return result;
}

/**
 * Check if head of repository is detached
 *
 * @param repo S4 class git_repository
 * @return TRUE if detached else FALSE
 */
SEXP git2r_repository_head_detached(SEXP repo)
{
    SEXP result;
    int head_detached;
    git_repository *repository;

    repository= git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    head_detached = git_repository_head_detached(repository);
    git_repository_free(repository);
    if (head_detached < 0)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    PROTECT(result = allocVector(LGLSXP, 1));
    if (1 == head_detached)
        LOGICAL(result)[0] = 1;
    else
        LOGICAL(result)[0] = 0;
    UNPROTECT(1);

    return result;
}

/**
 * Check if repository is empty.
 *
 * @param repo S4 class git_repository
 * @return TRUE if empty else FALSE
 */
SEXP git2r_repository_is_empty(SEXP repo)
{
    SEXP result;
    int is_empty;
    git_repository *repository;

    repository= git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    is_empty = git_repository_is_empty(repository);
    git_repository_free(repository);
    if (is_empty < 0)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    PROTECT(result = allocVector(LGLSXP, 1));
    if (1 == is_empty)
        LOGICAL(result)[0] = 1;
    else
        LOGICAL(result)[0] = 0;
    UNPROTECT(1);

    return result;
}

/**
 * Check if valid repository.
 *
 * @param path The path to the potential repository
 * @return TRUE if the repository can be opened else FALSE
 */
SEXP git2r_repository_can_open(SEXP path)
{
    SEXP result;
    int can_open;
    git_repository *repository = NULL;

    if (GIT_OK != git2r_arg_check_string(path))
        git2r_error(git2r_err_string_arg, __func__, "path");

    can_open = git_repository_open(&repository, CHAR(STRING_ELT(path, 0)));
    if (repository)
        git_repository_free(repository);

    PROTECT(result = allocVector(LGLSXP, 1));
    if (0 != can_open)
        LOGICAL(result)[0] = 0;
    else
        LOGICAL(result)[0] = 1;
    UNPROTECT(1);

    return result;
}

/**
 * Get workdir of repository.
 *
 * @param repo S4 class git_repository
 * @return R_NilValue if bare repository, else character vector
 * of length one with path.
 */
SEXP git2r_repository_workdir(SEXP repo)
{
    SEXP result = R_NilValue;
    git_repository *repository;

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    if (!git_repository_is_bare(repository)) {
        const char *wd = git_repository_workdir(repository);
        PROTECT(result = allocVector(STRSXP, 1));
        SET_STRING_ELT(result, 0, mkChar(wd));
        UNPROTECT(1);
    }

    git_repository_free(repository);

    return result;
}

/**
 * Find repository base path for given path
 *
 * @param path A character vector specifying the path to a file or folder
 * @return R_NilValue if repository cannot be found or
 * a character vector of length one with path to repository's git dir
 * e.g. /path/to/my/repo/.git
 */
SEXP git2r_repository_discover(SEXP path)
{
    int err;
    SEXP result = R_NilValue;
    git_buf buf = GIT_BUF_INIT;

    if (GIT_OK != git2r_arg_check_string(path))
        git2r_error(git2r_err_string_arg, __func__, "path");

    /* note that across_fs (arg #3) is set to 0 so this will stop when
     * a filesystem device change is detected while exploring parent
     * directories */
    err = git_repository_discover(&buf,
                                  CHAR(STRING_ELT(path, 0)),
                                  0,
                                  /* const char *ceiling_dirs */ NULL);
    if (GIT_OK != err) {
        /* NB just return R_NilValue if we can't discover the repo */
        if (GIT_ENOTFOUND == err)
            err = GIT_OK;
        goto cleanup;
    }

    PROTECT(result = allocVector(STRSXP, 1));
    SET_STRING_ELT(result, 0, mkChar(buf.ptr));

cleanup:
    git_buf_free(&buf);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (GIT_OK != err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return result;
}
