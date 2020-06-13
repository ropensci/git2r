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
 * Lookup an object in a repository
 *
 * @param repo S3 class git_repository
 * @param sha 4 to 40 char hexadecimal string
 * @return S3 object with lookup
 */
SEXP attribute_hidden
git2r_object_lookup(
    SEXP repo,
    SEXP sha)
{
    int error, nprotect = 0;
    size_t len;
    SEXP result = R_NilValue;
    git_object *object = NULL;
    git_oid oid;
    git_repository *repository = NULL;

    if (git2r_arg_check_sha(sha))
        git2r_error(__func__, NULL, "'sha'", git2r_err_sha_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    len = LENGTH(STRING_ELT(sha, 0));
    if (GIT_OID_HEXSZ == len) {
        git_oid_fromstr(&oid, CHAR(STRING_ELT(sha, 0)));
        error = git_object_lookup(&object, repository, &oid, GIT2R_OBJECT_ANY);
        if (error)
            goto cleanup;
    } else {
        git_oid_fromstrn(&oid, CHAR(STRING_ELT(sha, 0)), len);
        error = git_object_lookup_prefix(&object, repository, &oid, len, GIT2R_OBJECT_ANY);
        if (error)
            goto cleanup;
    }

    switch (git_object_type(object)) {
    case GIT2R_OBJECT_COMMIT:
        PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_commit));
        nprotect++;
        Rf_setAttrib(result, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_commit));
        git2r_commit_init((git_commit*)object, repo, result);
        break;
    case GIT2R_OBJECT_TREE:
        PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_tree));
        nprotect++;
        Rf_setAttrib(result, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_tree));
        git2r_tree_init((git_tree*)object, repo, result);
        break;
    case GIT2R_OBJECT_BLOB:
        PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_blob));
        nprotect++;
        Rf_setAttrib(result, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_blob));
        git2r_blob_init((git_blob*)object, repo, result);
        break;
    case GIT2R_OBJECT_TAG:
        PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_tag));
        nprotect++;
        Rf_setAttrib(result, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_tag));
        git2r_tag_init((git_tag*)object, repo, result);
        break;
    default:
        git2r_error(__func__, NULL, git2r_err_object_type, NULL);
    }

cleanup:
    git_object_free(object);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}
