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
#include "git2r_deprecated.h"
#include "git2r_error.h"
#include "git2r_index.h"
#include "git2r_repository.h"

/**
 * Add or update index entries matching files in the working
 * directory.
 *
 * @param repo S3 class git_repository
 * @param path array of path patterns
 * @param force if TRUE, add ignored files.
 * @return R_NilValue
 */
SEXP attribute_hidden
git2r_index_add_all(
    SEXP repo,
    SEXP path,
    SEXP force)
{
    int error = 0;
    unsigned int flags = 0;
    git_strarray pathspec = {0};
    git_index *index = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_string_vec(path))
        git2r_error(__func__, NULL, "'path'", git2r_err_string_vec_arg);
    if (git2r_arg_check_logical(force))
        git2r_error(__func__, NULL, "'force'", git2r_err_logical_arg);

    repository= git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git2r_copy_string_vec(&pathspec, path);
    if (error || !pathspec.count)
        goto cleanup;

    error = git_repository_index(&index, repository);
    if (error)
        goto cleanup;

    if (LOGICAL(force)[0])
        flags |= GIT_INDEX_ADD_FORCE;

    error = git_index_add_all(index, &pathspec, flags, NULL, NULL);
    if (error)
        goto cleanup;

    error = git_index_write(index);

cleanup:
    free(pathspec.strings);
    git_index_free(index);
    git_repository_free(repository);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return R_NilValue;
}

/**
 * Remove an index entry corresponding to a file relative to the
 * repository's working folder.
 *
 * @param repo S3 class git_repository
 * @param path array of path patterns
 * @return R_NilValue
 */
SEXP attribute_hidden
git2r_index_remove_bypath(
    SEXP repo,
    SEXP path)
{
    int error = 0;
    size_t i, len;
    git_index *index = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_string_vec(path))
        git2r_error(__func__, NULL, "'path'", git2r_err_string_vec_arg);

    len = Rf_length(path);
    if (!len)
        goto cleanup;

    repository= git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git_repository_index(&index, repository);
    if (error)
        goto cleanup;

    for (i = 0; i < len; i++) {
        if (NA_STRING != STRING_ELT(path, i)) {
            error = git_index_remove_bypath(index, CHAR(STRING_ELT(path, i)));
            if (error)
                goto cleanup;
        }
    }

    error = git_index_write(index);

cleanup:
    git_index_free(index);
    git_repository_free(repository);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return R_NilValue;
}
