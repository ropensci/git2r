/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2018 The git2r contributors
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

#include <string.h>

#include "git2r_objects.h"

const char *git2r_S3_class__git_merge_result = "git_merge_result";
const char *git2r_S3_items__git_merge_result[] = {
    "up_to_date", "fast_forward", "conflicts", "sha", ""};

/**
 * Get the list element named str, or return NULL.
 *
 * From the manual 'Writing R Extensions'
 * (https://cran.r-project.org/doc/manuals/r-release/R-exts.html)
 */
SEXP getListElement(SEXP list, const char *str)
{
    int i = 0;
    SEXP elmt = R_NilValue, names = Rf_getAttrib(list, R_NamesSymbol);

    for (; i < Rf_length(list); i++) {
        if(strcmp(CHAR(STRING_ELT(names, i)), str) == 0) {
            elmt = VECTOR_ELT(list, i);
            break;
        }
    }

    return elmt;
}
