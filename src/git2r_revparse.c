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
#include "git2.h"

#include "git2r_arg.h"
#include "git2r_commit.h"
#include "git2r_error.h"
#include "git2r_repository.h"
#include "git2r_tag.h"
#include "git2r_tree.h"

/**
 * Find object specified by revision
 *
 * @param repo S4 class git_repository
 * @param revision The revision string, see
 * http://git-scm.com/docs/git-rev-parse.html#_specifying_revisions
 * @return S4 object of class git_commit, git_tag or git_tree.
 */
SEXP git2r_revparse_single(SEXP repo, SEXP revision)
{
    int err;
    SEXP result = R_NilValue;
    git_repository *repository = NULL;
    git_object *treeish = NULL;

    if (git2r_arg_check_string(revision))
        git2r_error(__func__, NULL, "'revision'", git2r_err_string_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    err = git_revparse_single(
        &treeish,
        repository,
        CHAR(STRING_ELT(revision, 0)));
    if (err)
        goto cleanup;

    switch (git_object_type(treeish)) {
    case GIT_OBJ_COMMIT:
        PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_commit")));
        git2r_commit_init((git_commit*)treeish, repo, result);
        break;
    case GIT_OBJ_TAG:
        PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_tag")));
        git2r_tag_init((git_tag*)treeish, repo, result);
        break;
    case GIT_OBJ_TREE:
        PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_tree")));
        git2r_tree_init((git_tree*)treeish, repo, result);
        break;
    default:
        giterr_set_str(GITERR_NONE, git2r_err_revparse_single);
        err = GIT_ERROR;
        break;
    }

cleanup:
    if (treeish)
        git_object_free(treeish);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err) {
        if (GIT_ENOTFOUND == err) {
            git2r_error(
                __func__,
                NULL,
                git2r_err_revparse_not_found,
                NULL);
        } else {
            git2r_error(
                __func__,
                giterr_last(),
                NULL,
                NULL);
        }
    }

    return result;
}
