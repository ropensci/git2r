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

#include "git2r_oid.h"

/**
 * Get oid from hex SEXP
 *
 * @param hex
 * @param oid
 * @return void
 */
void git2r_oid_from_hex_sexp(SEXP hex, git_oid *oid)
{
    int err;
    size_t len;

    len = LENGTH(STRING_ELT(hex, 0));
    if (GIT_OID_HEXSZ == len)
        git_oid_fromstr(oid, CHAR(STRING_ELT(hex, 0)));
    else
        git_oid_fromstrn(oid, CHAR(STRING_ELT(hex, 0)), len);
}
