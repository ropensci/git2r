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

#include "git2r_blob.h"
#include "git2r_error.h"
#include "git2r_repository.h"
#include "git2r_tag.h"
#include "git2r_tree.h"

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
    char *class;
    git_repository *repository;

    if (R_NilValue == repo || S4SXP != TYPEOF(repo))
        return NULL;

    class_name = getAttrib(repo, R_ClassSymbol);
    if (check_string_arg(class_name))
        return NULL;

    class = CHAR(STRING_ELT(class_name, 0));
    if ((0 == strcmp(class, "git_repository"))
        && (0 == strcmp(class, "git_blob")))
        return NULL;

    path = GET_SLOT(repo, Rf_install("path"));
    if (check_string_arg(path))
        return NULL;

    if (git_repository_open(&repository, CHAR(STRING_ELT(path, 0))) < 0)
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

    if (check_string_arg(path) || check_logical_arg(bare))
        error("Invalid arguments to init");

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

    if (check_string_arg(path))
        error("Invalid arguments to is_repository");

    if (git_repository_open(&repository, CHAR(STRING_ELT(path, 0))) < 0)
        return ScalarLogical(FALSE);

    git_repository_free(repository);

    return ScalarLogical(TRUE);
}

/**
 * Lookup an object in a repository
 *
 * @param repo S4 class git_repository
 * @param hex 40 char hexadecimal string
 * @return S4 object with lookup
 */
SEXP lookup(const SEXP repo, const SEXP hex)
{
    int err;
    SEXP result = R_NilValue;
    git_object *object = NULL;
    git_oid oid;
    git_repository *repository = NULL;

    if (check_string_arg(hex))
        error("Invalid arguments to lookup");

    repository = get_repository(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    git_oid_fromstr(&oid, CHAR(STRING_ELT(hex, 0)));

    err = git_object_lookup(&object, repository, &oid, GIT_OBJ_ANY);
    if (err < 0)
        goto cleanup;

    switch (git_object_type(object)) {
    case GIT_OBJ_COMMIT:
        PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_commit")));
        init_commit((git_commit*)object, result);
        break;
    case GIT_OBJ_TREE:
        PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_tree")));
        init_tree((git_tree*)object, result);
        break;
    case GIT_OBJ_BLOB:
        PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_blob")));
        init_blob((git_blob*)object, repo, result);
        break;
    case GIT_OBJ_TAG:
        PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_tag")));
        init_tag((git_tag*)object, result);
        break;
    default:
        error("Unimplemented");
    }

cleanup:
    if (object)
        git_object_free(object);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return result;
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
