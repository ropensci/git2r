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
 * @param hex 4 to 40 char hexadecimal string
 * @return S4 object with lookup
 */
SEXP git2r_object_lookup(SEXP repo, SEXP hex)
{
    int err;
    size_t len;
    SEXP result = R_NilValue;
    git_object *object = NULL;
    git_oid oid;
    git_repository *repository = NULL;

    if (0 != git2r_arg_check_hex(hex))
        Rf_error("Invalid arguments to git2r_object_lookup");

    repository = git2r_repository_open(repo);
    if (!repository)
        Rf_error(git2r_err_invalid_repository);

    len = LENGTH(STRING_ELT(hex, 0));
    if (GIT_OID_HEXSZ == len) {
        git_oid_fromstr(&oid, CHAR(STRING_ELT(hex, 0)));
        err = git_object_lookup(&object, repository, &oid, GIT_OBJ_ANY);
        if (err < 0)
            goto cleanup;
    } else {
        git_oid_fromstrn(&oid, CHAR(STRING_ELT(hex, 0)), len);
        err = git_object_lookup_prefix(&object, repository, &oid, len, GIT_OBJ_ANY);
        if (err < 0)
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
        Rf_error("Unimplemented");
    }

cleanup:
    if (object)
        git_object_free(object);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err < 0)
        Rf_error("Error: %s\n", giterr_last()->message);

    return result;
}
