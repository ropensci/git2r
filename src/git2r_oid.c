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
#include "git2r_oid.h"

/**
 * Get oid from sha SEXP
 *
 * @param sha A character vector with sha's. The length
 * can be less than 40 bytes.
 * @param oid result is written into the oid
 * @return void
 */
void attribute_hidden
git2r_oid_from_sha_sexp(
    SEXP sha,
    git_oid *oid)
{
    size_t len;

    len = LENGTH(STRING_ELT(sha, 0));
    if (GIT_OID_HEXSZ == len)
        git_oid_fromstr(oid, CHAR(STRING_ELT(sha, 0)));
    else
        git_oid_fromstrn(oid, CHAR(STRING_ELT(sha, 0)), len);
}
