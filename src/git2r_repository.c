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

#include "git2r_error.h"
#include "git2r_repository.h"

/**
 * Get repo slot from S4 class git_repository
 *
 * @param repo S4 class git_repository
 * @return a git_repository pointer on success else NULL
 */
git_repository* get_repository(const SEXP repo)
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
    if (R_NilValue == path
        || !isString(path)
        || 1 != length(path)
        || NA_STRING == STRING_ELT(path, 0)
        || git_repository_open(&repository, CHAR(STRING_ELT(path, 0))) < 0)
        return NULL;

    return repository;
}

/**
 * Init a repository.
 *
 * @param path
 * @param bare
 * @return R_NilValue
 */
SEXP init(const SEXP path, const SEXP bare)
{
    int err;
    git_repository *repository = NULL;

    if (R_NilValue == path)
        error("'path' equals R_NilValue");
    if (!isString(path))
        error("'path' must be a string");
    if (R_NilValue == bare)
        error("'bare' equals R_NilValue");
    if (!isLogical(bare))
        error("'bare' must be a logical");

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
 * @return
 */
SEXP is_bare(const SEXP repo)
{
    SEXP result;
    git_repository *repository;

    repository= get_repository(repo);
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
 * Check if repository is empty.
 *
 * @param repo S4 class git_repository
 * @return
 */
SEXP is_empty(const SEXP repo)
{
    SEXP result;
    git_repository *repository;

    repository= get_repository(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    if (git_repository_is_empty(repository))
        result = ScalarLogical(TRUE);
    else
        result = ScalarLogical(FALSE);

    git_repository_free(repository);

    return result;
}

/**
 * Check if valid repository.
 *
 * @param path
 * @return
 */
SEXP is_repository(const SEXP path)
{
    git_repository *repository = NULL;

    if (R_NilValue == path
        || !isString(path)
        || 1 != length(path)
        || NA_STRING == STRING_ELT(path, 0))
        error("Invalid arguments to is_repository");

    if (git_repository_open(&repository, CHAR(STRING_ELT(path, 0))) < 0)
        return ScalarLogical(FALSE);

    git_repository_free(repository);

    return ScalarLogical(TRUE);
}

/**
 * Get workdir of repository.
 *
 * @param repo S4 class git_repository
 * @return R_NilValue if bare repository, else character vector with path.
 */
SEXP workdir(const SEXP repo)
{
    SEXP result = R_NilValue;
    git_repository *repository;

    repository = get_repository(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    if (!git_repository_is_bare(repository))
        result = ScalarString(mkChar(git_repository_workdir(repository)));

    git_repository_free(repository);

    return result;
}
