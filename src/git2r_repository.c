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
    if (git2r_arg_check_string(path))
        return NULL;

    if (git_repository_open(&repository, CHAR(STRING_ELT(path, 0))) < 0)
        return NULL;

    return repository;
}

/**
 * Get head of repository
 *
 * @param repo S4 class git_repository
 * @return R_NilValue if unborn branch or not found.
           S4 class git_branch if not a detached head.
           S4 class git_commit if detached head
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
        error(git2r_err_invalid_repository);

    err = git_repository_head(&reference, repository);
    if (err < 0) {
        if (GIT_EUNBORNBRANCH == err || GIT_ENOTFOUND == err)
            err = 0;
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
        if (err < 0)
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

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return result;
}

/**
 * Init a repository.
 *
 * @param path :TODO:DOCUMENTATION:
 * @param bare :TODO:DOCUMENTATION:
 * @return R_NilValue :TODO:DOCUMENTATION:
 */
SEXP git2r_repository_init(SEXP path, SEXP bare)
{
    int err;
    git_repository *repository = NULL;

    if (git2r_arg_check_string(path) || git2r_arg_check_logical(bare))
        error("Invalid arguments to git2r_repository_init");

    err = git_repository_init(&repository,
                              CHAR(STRING_ELT(path, 0)),
                              LOGICAL(bare)[0]);
    if (err < 0)
        error("Unable to init repository");

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
    git_repository *repository;

    repository= git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    if (git_repository_is_bare(repository))
        result = ScalarLogical(TRUE);
    else
        result = ScalarLogical(FALSE);

    git_repository_free(repository);

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
    git_repository *repository;

    repository= git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    if (git_repository_is_shallow(repository))
        result = ScalarLogical(TRUE);
    else
        result = ScalarLogical(FALSE);

    git_repository_free(repository);

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
    int result;
    git_repository *repository;

    repository= git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);
    result = git_repository_head_detached(repository);
    git_repository_free(repository);
    if (result < 0)
        error("Error: %s\n", giterr_last()->message);
    if (1 == result)
        return ScalarLogical(TRUE);
    return ScalarLogical(FALSE);
}

/**
 * Check if repository is empty.
 *
 * @param repo S4 class git_repository
 * @return :TODO:DOCUMENTATION:
 */
SEXP git2r_repository_is_empty(SEXP repo)
{
    int result;
    git_repository *repository;

    repository= git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);
    result = git_repository_is_empty(repository);
    git_repository_free(repository);
    if (result < 0)
        error("Error: %s\n", giterr_last()->message);
    if (1 == result)
        return ScalarLogical(TRUE);
    return ScalarLogical(FALSE);
}

/**
 * Check if valid repository.
 *
 * @param path :TODO:DOCUMENTATION:
 * @return :TODO:DOCUMENTATION:
 */
SEXP git2r_repository_can_open(SEXP path)
{
    git_repository *repository = NULL;

    if (git2r_arg_check_string(path))
        error("Invalid arguments to git2r_repository_can_open");

    if (git_repository_open(&repository, CHAR(STRING_ELT(path, 0))) < 0)
        return ScalarLogical(FALSE);

    git_repository_free(repository);

    return ScalarLogical(TRUE);
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
        error(git2r_err_invalid_repository);

    if (!git_repository_is_bare(repository))
        result = ScalarString(mkChar(git_repository_workdir(repository)));

    git_repository_free(repository);

    return result;
}

/**
 * Find repository base path for given path
 *
 * @param path :TODO:DOCUMENTATION:
 * @return R_NilValue if repository cannot be found or
 * a character vector of length one with path to repository's git dir
 * e.g. /path/to/my/repo/.git
 */
SEXP git2r_repository_discover(SEXP startpath)
{
    int err;
    SEXP result = R_NilValue;
    git_buf gitdir = GIT_BUF_INIT;

    if (git2r_arg_check_string(startpath))
        error("Invalid arguments to git2r_repository_discover");

    /* note that across_fs (arg #3) is set to 0 so this will stop when a
       filesystem device change is detected while exploring parent directories
    */
    err = git_repository_discover(&gitdir, CHAR(STRING_ELT(startpath, 0)), 0,
    /* const char *ceiling_dirs */ NULL);
    if (err < 0) {
        /* NB just return R_NilValue if we can't discover the repo */
        if (GIT_ENOTFOUND == err)
            err = 0;
        goto cleanup;
    }

    result = ScalarString(mkChar(gitdir.ptr));

cleanup:
    git_buf_free(&gitdir);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return result;
}
