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
#include "git2r_blob.h"
#include "git2r_error.h"

/**
 * Init slots in S4 class git_blob
 *
 * @param source a blob
 * @param dest S4 class git_blob to initialize
 * @return void
 */
void init_blob(const git_blob *source, SEXP dest)
{
    char hex[GIT_OID_HEXSZ + 1];

    git_oid_fmt(hex, git_blob_id(source));
    hex[GIT_OID_HEXSZ] = '\0';
    SET_SLOT(dest,
             Rf_install("hex"),
             ScalarString(mkChar(hex)));
}

/**
 * Lookup a blob object from a repository
 *
 * @param repo S4 class git_repository
 * @param id
 * @return S4 objects of class git_blob
 */
SEXP lookup(const SEXP repo, const SEXP id)
{
    int err;
    SEXP sexp_blob = R_NilValue;
    git_blob *blob = NULL;
    git_oid oid;
    git_repository *repository = NULL;

    if (check_string_arg(id))
        error("Invalid arguments to lookup");

    repository = get_repository(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    git_oid_fromstr(&oid, CHAR(STRING_ELT(id, 0)));

    err = git_blob_lookup(&blob, repository, &oid);
    if (err < 0)
        goto cleanup;

    PROTECT(sexp_blob = NEW_OBJECT(MAKE_CLASS("git_blob")));
    init_blob(blob, sexp_blob);

cleanup:
    if (blob)
        git_blob_free(blob);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != sexp_blob)
        UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return sexp_blob;
}
