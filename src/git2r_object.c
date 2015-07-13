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
#include "git2r_blob.h"
#include "git2r_commit.h"
#include "git2r_error.h"
#include "git2r_repository.h"
#include "git2r_tag.h"
#include "git2r_tree.h"

/**
 * Lookup an object in a repository
 *
 * @param repo S4 class git_repository
 * @param sha 4 to 40 char hexadecimal string
 * @return S4 object with lookup
 */
SEXP git2r_object_lookup(SEXP repo, SEXP sha)
{
    int err;
    size_t len;
    SEXP result = R_NilValue;
    git_object *object = NULL;
    git_oid oid;
    git_repository *repository = NULL;

    if (git2r_arg_check_sha(sha))
        git2r_error(git2r_err_sha_arg, __func__, "sha");

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    len = LENGTH(STRING_ELT(sha, 0));
    if (GIT_OID_HEXSZ == len) {
        git_oid_fromstr(&oid, CHAR(STRING_ELT(sha, 0)));
        err = git_object_lookup(&object, repository, &oid, GIT_OBJ_ANY);
        if (err)
            goto cleanup;
    } else {
        git_oid_fromstrn(&oid, CHAR(STRING_ELT(sha, 0)), len);
        err = git_object_lookup_prefix(&object, repository, &oid, len, GIT_OBJ_ANY);
        if (err)
            goto cleanup;
    }

    switch (git_object_type(object)) {
    case GIT_OBJ_COMMIT:
        PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_commit")));
        git2r_commit_init((git_commit*)object, repo, result);
        break;
    case GIT_OBJ_TREE:
        PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_tree")));
        git2r_tree_init((git_tree*)object, repo, result);
        break;
    case GIT_OBJ_BLOB:
        PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_blob")));
        git2r_blob_init((git_blob*)object, repo, result);
        break;
    case GIT_OBJ_TAG:
        PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_tag")));
        git2r_tag_init((git_tag*)object, repo, result);
        break;
    default:
        git2r_error("Error in '%s': Unimplemented object type. Sorry.", __func__, NULL);
    }

cleanup:
    if (object)
        git_object_free(object);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return result;
}
