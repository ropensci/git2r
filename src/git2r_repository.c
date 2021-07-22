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
#include "git2r_blob.h"
#include "git2r_branch.h"
#include "git2r_commit.h"
#include "git2r_deprecated.h"
#include "git2r_error.h"
#include "git2r_repository.h"
#include "git2r_S3.h"
#include "git2r_signature.h"
#include "git2r_tag.h"
#include "git2r_tree.h"

/**
 * Get repo from S3 class git_repository
 *
 * @param repo S3 class git_repository
 * @return a git_repository pointer on success else NULL
 */
attribute_hidden git_repository*
git2r_repository_open(
    SEXP repo)
{
    int error;
    SEXP path;
    git_repository *repository = NULL;

    if (git2r_arg_check_repository(repo)) {
        Rprintf("The repo argument is unexpectedly invalid\n");
        return NULL;
    }

    path = git2r_get_list_element(repo, "path");
    error = git_repository_open(&repository, CHAR(STRING_ELT(path, 0)));
    if (error) {
        if (error == GIT_ENOTFOUND)
            Rf_warning("Could not find repository at path '%s'", CHAR(STRING_ELT(path, 0)));
        else
            Rf_warning("Unable to open repository: %s", GIT2R_ERROR_LAST()->message);
        git_repository_free(repository);
        return NULL;
    }

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
static int
git2r_repository_fetchhead_foreach_cb(
    const char *ref_name,
    const char *remote_url,
    const git_oid *oid,
    unsigned int is_merge,
    void *payload)
{
    git2r_fetch_head_cb_data *cb_data = (git2r_fetch_head_cb_data*)payload;

    /* Check if we have a list to populate */
    if (!Rf_isNull(cb_data->list)) {
        char sha[GIT_OID_HEXSZ + 1];
        SEXP fetch_head;

        PROTECT(fetch_head = Rf_mkNamed(VECSXP, git2r_S3_items__git_fetch_head));
        Rf_setAttrib(
            fetch_head,
            R_ClassSymbol,
            Rf_mkString(git2r_S3_class__git_fetch_head));

        SET_VECTOR_ELT(
            fetch_head,
            git2r_S3_item__git_fetch_head__ref_name,
            Rf_mkString(ref_name));

        SET_VECTOR_ELT(
            fetch_head,
            git2r_S3_item__git_fetch_head__remote_url,
            Rf_mkString(remote_url));

        git_oid_tostr(sha, sizeof(sha), oid);
        SET_VECTOR_ELT(
            fetch_head,
            git2r_S3_item__git_fetch_head__sha,
            Rf_mkString(sha));

        SET_VECTOR_ELT(
            fetch_head,
            git2r_S3_item__git_fetch_head__is_merge,
            Rf_ScalarLogical(is_merge));

        SET_VECTOR_ELT(
            fetch_head,
            git2r_S3_item__git_fetch_head__repo,
            Rf_duplicate(cb_data->repo));

        SET_VECTOR_ELT(cb_data->list, cb_data->n, fetch_head);
        UNPROTECT(1);
    }

    cb_data->n += 1;

    return 0;
}

/**
 * Get entries in FETCH_HEAD file
 *
 * @param repo S3 class git_repository
 * @return list with the S3 class git_fetch_head entries. R_NilValue
 * if there is no FETCH_HEAD file.
 */
SEXP attribute_hidden
git2r_repository_fetch_heads(
    SEXP repo)
{
    int error, nprotect = 0;
    SEXP result = R_NilValue;
    git2r_fetch_head_cb_data cb_data = {0, R_NilValue, R_NilValue};
    git_repository *repository = NULL;

    repository= git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    /* Count number of fetch heads before creating the list */
    error = git_repository_fetchhead_foreach(
        repository,
        git2r_repository_fetchhead_foreach_cb,
        &cb_data);

    if (error) {
        if (GIT_ENOTFOUND == error)
            error = GIT_OK;
        goto cleanup;
    }

    PROTECT(result = Rf_allocVector(VECSXP, cb_data.n));
    nprotect++;
    cb_data.n = 0;
    cb_data.list = result;
    cb_data.repo = repo;
    error = git_repository_fetchhead_foreach(
        repository,
        git2r_repository_fetchhead_foreach_cb,
        &cb_data);

cleanup:
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Get head of repository
 *
 * @param repo S3 class git_repository
 * @return R_NilValue if unborn branch or not found. S3 class
 * git_branch if not a detached head. S3 class git_commit if detached
 * head
 */
SEXP attribute_hidden
git2r_repository_head(
    SEXP repo)
{
    int error, nprotect = 0;
    SEXP result = R_NilValue;
    git_commit *commit = NULL;
    git_reference *reference = NULL;
    git_repository *repository = NULL;

    repository= git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git_repository_head(&reference, repository);
    if (error) {
        if (GIT_EUNBORNBRANCH == error || GIT_ENOTFOUND == error)
            error = GIT_OK;
        goto cleanup;
    }

    if (git_reference_is_branch(reference)) {
        git_branch_t type = GIT_BRANCH_LOCAL;
        if (git_reference_is_remote(reference))
            type = GIT_BRANCH_REMOTE;
        PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_branch));
        nprotect++;
        Rf_setAttrib(result, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_branch));
        error = git2r_branch_init(reference, type, repo, result);
    } else {
        error = git_commit_lookup(
            &commit,
            repository,
            git_reference_target(reference));
        if (error)
            goto cleanup;
        PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_commit));
        nprotect++;
        Rf_setAttrib(result, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_commit));
        git2r_commit_init(commit, repo, result);
    }

cleanup:
    git_commit_free(commit);
    git_reference_free(reference);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

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
 * @param branch Use the specified name for the initial branch in the
 * newly created repository. If NULL, fall back to the default name.
 * @return R_NilValue
 */
SEXP attribute_hidden
git2r_repository_init(
    SEXP path,
    SEXP bare,
    SEXP branch)
{
    int error;
    git_repository *repository = NULL;
    git_repository_init_options opts = GIT_REPOSITORY_INIT_OPTIONS_INIT;

    if (git2r_arg_check_string(path))
        git2r_error(__func__, NULL, "'path'", git2r_err_string_arg);
    if (git2r_arg_check_logical(bare))
        git2r_error(__func__, NULL, "'bare'", git2r_err_logical_arg);
    if (!Rf_isNull(branch) && git2r_arg_check_string(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_string_arg);

    if (LOGICAL(bare)[0])
        opts.flags |= GIT_REPOSITORY_INIT_BARE;
    if (!Rf_isNull(branch))
        opts.initial_head = CHAR(STRING_ELT(branch, 0));

    error = git_repository_init_ext(&repository,
                                    CHAR(STRING_ELT(path, 0)),
                                    &opts);
    if (error)
        git2r_error(__func__, NULL, git2r_err_repo_init, NULL);

    git_repository_free(repository);

    return R_NilValue;
}

/**
 * Check if repository is bare.
 *
 * @param repo S3 class git_repository
 * @return TRUE if bare else FALSE
 */
SEXP attribute_hidden
git2r_repository_is_bare(
    SEXP repo)
{
    int is_bare;
    git_repository *repository;

    repository= git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    is_bare = git_repository_is_bare(repository);
    git_repository_free(repository);
    if (is_bare < 0)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);
    return Rf_ScalarLogical(is_bare);
}

/**
 * Determine if the repository was a shallow clone.
 *
 * @param repo S3 class git_repository
 * @return TRUE if shallow else FALSE
 */
SEXP attribute_hidden
git2r_repository_is_shallow(
    SEXP repo)
{
    int is_shallow;
    git_repository *repository;

    repository= git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    is_shallow = git_repository_is_shallow(repository);
    git_repository_free(repository);
    if (is_shallow < 0)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);
    return Rf_ScalarLogical(is_shallow);
}

/**
 * Check if head of repository is detached
 *
 * @param repo S3 class git_repository
 * @return TRUE if detached else FALSE
 */
SEXP attribute_hidden
git2r_repository_head_detached(
    SEXP repo)
{
    int head_detached;
    git_repository *repository;

    repository= git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    head_detached = git_repository_head_detached(repository);
    git_repository_free(repository);
    if (head_detached < 0)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);
    return Rf_ScalarLogical(head_detached);
}

/**
 * Check if repository is empty.
 *
 * @param repo S3 class git_repository
 * @return TRUE if empty else FALSE
 */
SEXP attribute_hidden
git2r_repository_is_empty(
    SEXP repo)
{
    int is_empty;
    git_repository *repository;

    repository= git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    is_empty = git_repository_is_empty(repository);
    git_repository_free(repository);
    if (is_empty < 0)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);
    return Rf_ScalarLogical(is_empty);
}

/**
 * Check if valid repository.
 *
 * @param path The path to the potential repository
 * @return TRUE if the repository can be opened else FALSE
 */
SEXP attribute_hidden
git2r_repository_can_open(
    SEXP path)
{
    int error;
    git_repository *repository = NULL;

    if (git2r_arg_check_string(path))
        git2r_error(__func__, NULL, "'path'", git2r_err_string_arg);

    error = git_repository_open(&repository, CHAR(STRING_ELT(path, 0)));
    git_repository_free(repository);

    if (error)
        return Rf_ScalarLogical(0);
    return Rf_ScalarLogical(1);
}

/**
 * Make the repository HEAD point to the specified reference.
 *
 * @param repo S3 class git_repository
 * @param ref_name Canonical name of the reference the HEAD should point at
 * @return R_NilValue
 */
SEXP attribute_hidden
git2r_repository_set_head(
    SEXP repo,
    SEXP ref_name)
{
    int error;
    git_repository *repository = NULL;

    if (git2r_arg_check_string(ref_name))
        git2r_error(__func__, NULL, "'ref_name'", git2r_err_string_arg);
    if (!git_reference_is_valid_name(CHAR(STRING_ELT(ref_name, 0))))
        git2r_error(__func__, NULL, git2r_err_invalid_refname, NULL);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git_repository_set_head(repository, CHAR(STRING_ELT(ref_name, 0)));

    git_repository_free(repository);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return R_NilValue;
}

/**
 * Make the repository HEAD directly point to the commit.
 *
 * @param commit S3 class git_commit
 * @return R_NilValue
 */
SEXP attribute_hidden
git2r_repository_set_head_detached(
    SEXP commit)
{
    int error;
    SEXP sha;
    git_oid oid;
    git_commit *treeish = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_commit(commit))
        git2r_error(__func__, NULL, "'commit'", git2r_err_commit_arg);

    repository = git2r_repository_open(git2r_get_list_element(commit, "repo"));
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    sha = git2r_get_list_element(commit, "sha");
    error = git_oid_fromstr(&oid, CHAR(STRING_ELT(sha, 0)));
    if (error)
        goto cleanup;

    error = git_commit_lookup(&treeish, repository, &oid);
    if (error)
        goto cleanup;

    error = git_repository_set_head_detached(
        repository,
        git_commit_id(treeish));

cleanup:
    git_commit_free(treeish);
    git_repository_free(repository);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return R_NilValue;
}

/**
 * Get workdir of repository.
 *
 * @param repo S3 class git_repository
 * @return R_NilValue if bare repository, else character vector
 * of length one with path.
 */
SEXP attribute_hidden
git2r_repository_workdir(
    SEXP repo)
{
    int nprotect = 0;
    SEXP result = R_NilValue;
    git_repository *repository;

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    if (!git_repository_is_bare(repository)) {
        const char *wd = git_repository_workdir(repository);
        PROTECT(result = Rf_allocVector(STRSXP, 1));
        nprotect++;
        SET_STRING_ELT(result, 0, Rf_mkChar(wd));
    }

    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    return result;
}

/**
 * Find repository base path for given path
 *
 * @param path A character vector specifying the path to a file or folder
 * @param ceiling The lookup will stop when this absolute path is reached.
 * @return R_NilValue if repository cannot be found or
 * a character vector of length one with path to repository's git dir
 * e.g. /path/to/my/repo/.git
 */
SEXP attribute_hidden
git2r_repository_discover(
    SEXP path,
    SEXP ceiling)
{
    int error, nprotect = 0;
    SEXP result = R_NilValue;
    git_buf buf = GIT_BUF_INIT_CONST(NULL, 0);
    const char *ceiling_dirs = NULL;

    if (git2r_arg_check_string(path))
        git2r_error(__func__, NULL, "'path'", git2r_err_string_arg);
    if (!Rf_isNull(ceiling)) {
        if (git2r_arg_check_string(ceiling))
            git2r_error(__func__, NULL, "'ceiling'", git2r_err_string_arg);
        ceiling_dirs = CHAR(STRING_ELT(ceiling, 0));
    }

    /* note that across_fs (arg #3) is set to 0 so this will stop when
     * a filesystem device change is detected while exploring parent
     * directories */
    error = git_repository_discover(
        &buf,
        CHAR(STRING_ELT(path, 0)),
        0,
        ceiling_dirs);
    if (error) {
        /* NB just return R_NilValue if we can't discover the repo */
        if (GIT_ENOTFOUND == error)
            error = GIT_OK;
        goto cleanup;
    }

    PROTECT(result = Rf_allocVector(STRSXP, 1));
    nprotect++;
    SET_STRING_ELT(result, 0, Rf_mkChar(buf.ptr));

cleanup:
    GIT2R_BUF_DISPOSE(&buf);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}
