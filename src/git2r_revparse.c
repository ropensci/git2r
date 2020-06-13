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
#include "git2r_commit.h"
#include "git2r_deprecated.h"
#include "git2r_error.h"
#include "git2r_repository.h"
#include "git2r_S3.h"
#include "git2r_tag.h"
#include "git2r_tree.h"

/**
 * Find object specified by revision
 *
 * @param repo S3 class git_repository
 * @param revision The revision string, see
 * http://git-scm.com/docs/git-rev-parse.html#_specifying_revisions
 * @return S3 object of class git_commit, git_tag or git_tree.
 */
SEXP attribute_hidden
git2r_revparse_single(
    SEXP repo,
    SEXP revision)
{
    int error, nprotect = 0;
    SEXP result = R_NilValue;
    git_repository *repository = NULL;
    git_object *treeish = NULL;

    if (git2r_arg_check_string(revision))
        git2r_error(__func__, NULL, "'revision'", git2r_err_string_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git_revparse_single(
        &treeish,
        repository,
        CHAR(STRING_ELT(revision, 0)));
    if (error)
        goto cleanup;

    switch (git_object_type(treeish)) {
    case GIT2R_OBJECT_BLOB:
        PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_blob));
        nprotect++;
        Rf_setAttrib(result, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_blob));
        git2r_blob_init((git_blob*)treeish, repo, result);
        break;
    case GIT2R_OBJECT_COMMIT:
        PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_commit));
        nprotect++;
        Rf_setAttrib(result, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_commit));
        git2r_commit_init((git_commit*)treeish, repo, result);
        break;
    case GIT2R_OBJECT_TAG:
        PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_tag));
        nprotect++;
        Rf_setAttrib(result, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_tag));
        git2r_tag_init((git_tag*)treeish, repo, result);
        break;
    case GIT2R_OBJECT_TREE:
        PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_tree));
        nprotect++;
        Rf_setAttrib(result, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_tree));
        git2r_tree_init((git_tree*)treeish, repo, result);
        break;
    default:
        GIT2R_ERROR_SET_STR(GIT2R_ERROR_NONE, git2r_err_revparse_single);
        error = GIT_ERROR;
        break;
    }

cleanup:
    git_object_free(treeish);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error) {
        if (GIT_ENOTFOUND == error) {
            git2r_error(
                __func__,
                NULL,
                git2r_err_revparse_not_found,
                NULL);
        } else {
            git2r_error(
                __func__,
                GIT2R_ERROR_LAST(),
                NULL,
                NULL);
        }
    }

    return result;
}
