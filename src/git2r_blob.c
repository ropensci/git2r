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

/**
 * Init slots in S4 class git_blob
 *
 * @param source a blob
 * @param repo S4 class git_repository that contains the blob
 * @param dest S4 class git_blob to initialize
 * @return void
 */
void init_blob(const git_blob *source, SEXP repo, SEXP dest)
{
    const git_oid *oid;
    char hex[GIT_OID_HEXSZ + 1];

    oid = git_blob_id(source);
    git_oid_tostr(hex, sizeof(hex), oid);
    SET_SLOT(dest,
             Rf_install("hex"),
             ScalarString(mkChar(hex)));

    SET_SLOT(dest, Rf_install("repo"), duplicate(repo));
}

/**
 * Is blob binary
 *
 * @param blob S4 class git_blob
 * @return TRUE if binary data, FALSE if not
 */
SEXP is_binary(SEXP blob)
{
    int err;
    SEXP result = R_NilValue;
    SEXP hex;
    git_blob *blob_obj = NULL;
    git_oid oid;
    git_repository *repository = NULL;

    repository= get_repository(GET_SLOT(blob, Rf_install("repo")));
    if (!repository)
        error(git2r_err_invalid_repository);

    hex = GET_SLOT(blob, Rf_install("hex"));
    git_oid_fromstr(&oid, CHAR(STRING_ELT(hex, 0)));

    err = git_blob_lookup(&blob_obj, repository, &oid);
    if (err < 0)
        goto cleanup;

    PROTECT(result = allocVector(LGLSXP, 1));
    if (git_blob_is_binary(blob_obj))
        LOGICAL(result)[0] = 1;
    else
        LOGICAL(result)[0] = 0;

cleanup:
    if (blob_obj)
        git_blob_free(blob_obj);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return result;
}
