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
#include "git2r_reset.h"
#include "git2r_S3.h"
#include "git2r_signature.h"

/**
 * Reset current HEAD to the specified state
 *
 * @param commit The commit to which the HEAD should be moved to.
 * @param reset_type Kind of reset operation to perform. 'soft' means
 * the Head will be moved to the commit. 'mixed' reset will trigger a
 * 'soft' reset, plus the index will be replaced with the content of
 * the commit tree. 'hard' reset will trigger a 'mixed' reset and the
 * working directory will be replaced with the content of the index.
 * @return R_NilValue
 */
SEXP attribute_hidden
git2r_reset(
    SEXP commit,
    SEXP reset_type)
{
    int error;
    SEXP repo;
    git_commit *target = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_commit(commit))
        git2r_error(__func__, NULL, "'commit'", git2r_err_commit_arg);
    if (git2r_arg_check_integer(reset_type))
        git2r_error(__func__, NULL, "'reset_type'", git2r_err_integer_arg);

    repo = git2r_get_list_element(commit, "repo");
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git2r_commit_lookup(&target, repository, commit);
    if (error)
        goto cleanup;

    error = git_reset(
        repository,
        (git_object*)target,
        INTEGER(reset_type)[0],
        NULL);

cleanup:
    git_commit_free(target);
    git_repository_free(repository);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return R_NilValue;
}

/**
 * Updates some entries in the index from the HEAD commit tree.
 *
 * @param repo S3 class git_repository
 * @param path The paths to reset
 * @return R_NilValue
 */
SEXP attribute_hidden
git2r_reset_default(
    SEXP repo,
    SEXP path)
{
    int error = 0;
    git_strarray pathspec = {0};
    git_reference *head = NULL;
    git_object *head_commit = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_string_vec(path))
        git2r_error(__func__, NULL, "'path'", git2r_err_string_vec_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git2r_copy_string_vec(&pathspec, path);
    if (error || !pathspec.count)
        goto cleanup;

    error = git_repository_head(&head, repository);
    if (error)
        goto cleanup;

    error = git_reference_peel(&head_commit, head, GIT2R_OBJECT_COMMIT);
    if (error)
        goto cleanup;

    error = git_reset_default(repository, head_commit, &pathspec);

cleanup:
    git_reference_free(head);
    git_object_free(head_commit);
    free(pathspec.strings);
    git_repository_free(repository);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return R_NilValue;
}
