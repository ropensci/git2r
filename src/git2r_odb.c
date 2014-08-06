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

#include "git2.h"

#include "git2r_arg.h"
#include "git2r_error.h"
#include "git2r_odb.h"

/**
 * Determine the sha1 hex of character vectors without writing to the
 * object data base.
 *
 * @param data STRSXP with character vectors to hash
 * @return A STRSXP with character vector of sha1 hex values
 */
SEXP git2r_odb_hash(SEXP data)
{
    SEXP result;
    int err = 0;
    size_t len, i;
    char hex[GIT_OID_HEXSZ + 1];
    git_oid oid;

    if (0 != git2r_arg_check_string_vec(data))
        Rf_error(git2r_err_string_vec_arg, "data");

    len = length(data);
    PROTECT(result = allocVector(STRSXP, len));
    for (i = 0; i < len; i++) {
        if (NA_STRING == STRING_ELT(data, i)) {
            SET_STRING_ELT(result, i, NA_STRING);
        } else {
            err = git_odb_hash(&oid,
                               CHAR(STRING_ELT(data, i)),
                               LENGTH(STRING_ELT(data, i)),
                               GIT_OBJ_BLOB);
            if (err < 0)
                break;

            git_oid_fmt(hex, &oid);
            hex[GIT_OID_HEXSZ] = '\0';
            SET_STRING_ELT(result, i, mkChar(hex));
        }
    }

    UNPROTECT(1);

    if (err < 0)
        Rf_error("Error: %s\n", giterr_last()->message);

    return result;
}

/**
 * Determine the sha1 hex of files without writing to the object data
 * base.
 *
 * @param path STRSXP with file vectors to hash
 * @return A STRSXP with character vector of sha1 hex values
 */
SEXP git2r_odb_hashfile(SEXP path)
{
    SEXP result;
    int err = 0;
    size_t len, i;
    char hex[GIT_OID_HEXSZ + 1];
    git_oid oid;

    if (0 != git2r_arg_check_string_vec(path))
        Rf_error(git2r_err_string_vec_arg, "path");

    len = length(path);
    PROTECT(result = allocVector(STRSXP, len));
    for (i = 0; i < len; i++) {
        if (NA_STRING == STRING_ELT(path, i)) {
            SET_STRING_ELT(result, i, NA_STRING);
        } else {
            err = git_odb_hashfile(&oid,
                                   CHAR(STRING_ELT(path, i)),
                                   GIT_OBJ_BLOB);
            if (err < 0)
                break;

            git_oid_fmt(hex, &oid);
            hex[GIT_OID_HEXSZ] = '\0';
            SET_STRING_ELT(result, i, mkChar(hex));
        }
    }

    UNPROTECT(1);

    if (err < 0)
        Rf_error("Error: %s\n", giterr_last()->message);

    return result;
}
