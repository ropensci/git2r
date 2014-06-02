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
#include "git2r_error.h"

/**
 * Error messages
 */

const char git2r_err_alloc_memory_buffer[] = "Unable to allocate memory buffer";
const char git2r_err_invalid_repository[] = "Invalid repository";
const char git2r_err_nothing_added_to_commit[] = "Nothing added to commit";
const char git2r_err_unexpected_config_level[] = "Unexpected config level";
const char git2r_err_unexpected_type_of_branch[] = "Unexpected type of branch";
const char git2r_err_unexpected_head_of_branch[] = "Unexpected head of branch";

/**
 * Check blob argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else 1
 */
int git2r_error_check_blob_arg(SEXP arg)
{
    SEXP class_name;

    if (R_NilValue == arg || S4SXP != TYPEOF(arg))
        return 1;

    class_name = getAttrib(arg, R_ClassSymbol);
    if (0 != strcmp(CHAR(STRING_ELT(class_name, 0)), "git_blob"))
        return 1;

    if (git2r_error_check_string_arg(GET_SLOT(arg, Rf_install("hex"))))
        return 1;

    return 0;
}

/**
 * Check branch argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else 1
 */
int git2r_error_check_branch_arg(SEXP arg)
{
    SEXP class_name;
    SEXP slot;

    if (R_NilValue == arg || S4SXP != TYPEOF(arg))
        return 1;

    class_name = getAttrib(arg, R_ClassSymbol);
    if (0 != strcmp(CHAR(STRING_ELT(class_name, 0)), "git_branch"))
        return 1;

    slot = GET_SLOT(arg, Rf_install("branch_type"));
    if (git2r_error_check_integer_arg(slot))
        return 1;
    switch (INTEGER(slot)[0]) {
    case GIT_BRANCH_LOCAL:
    case GIT_BRANCH_REMOTE:
        break;
    default:
        return 1;
    }

    return 0;
}

/**
 * Check commit argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else 1
 */
int git2r_error_check_commit_arg(SEXP arg)
{
    SEXP class_name;

    if (R_NilValue == arg || S4SXP != TYPEOF(arg))
        return 1;

    class_name = getAttrib(arg, R_ClassSymbol);
    if (0 != strcmp(CHAR(STRING_ELT(class_name, 0)), "git_commit"))
        return 1;

    if (git2r_error_check_string_arg(GET_SLOT(arg, Rf_install("hex"))))
        return 1;

    return 0;
}

/**
 * Check hex argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else 1
 */
int git2r_error_check_hex_arg(SEXP arg)
{
    size_t len;

    if (git2r_error_check_string_arg(arg))
        return 1;

    len = LENGTH(STRING_ELT(arg, 0));
    if (len < GIT_OID_MINPREFIXLEN || len > GIT_OID_HEXSZ)
        return 1;

    return 0;
}

/**
 * Check integer argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else 1
 */
int git2r_error_check_integer_arg(SEXP arg)
{
    if (R_NilValue == arg
        || !isInteger(arg)
        || 1 != length(arg)
        || NA_INTEGER == INTEGER(arg)[0])
        return 1;
    return 0;
}

/**
 * Check logical argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else 1
 */
int git2r_error_check_logical_arg(SEXP arg)
{
    if (R_NilValue == arg
        || !isLogical(arg)
        || 1 != length(arg)
        || NA_LOGICAL == LOGICAL(arg)[0])
        return 1;
    return 0;
}

/**
 * Check real argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else 1
 */
int check_real_arg(SEXP arg)
{
    if (R_NilValue == arg
        || !isReal(arg)
        || 1 != length(arg)
        || NA_REAL == REAL(arg)[0])
        return 1;
    return 0;
}

/**
 * Check signature argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else 1
 */
int git2r_error_check_signature_arg(SEXP arg)
{
    SEXP class_name;
    SEXP when;

    if (R_NilValue == arg || S4SXP != TYPEOF(arg))
        return 1;

    class_name = getAttrib(arg, R_ClassSymbol);
    if (0 != strcmp(CHAR(STRING_ELT(class_name, 0)), "git_signature"))
        return 1;

    if (git2r_error_check_string_arg(GET_SLOT(arg, Rf_install("name")))
        || git2r_error_check_string_arg(GET_SLOT(arg, Rf_install("email"))))
        return 1;

    when = GET_SLOT(arg, Rf_install("when"));
    if (check_real_arg(GET_SLOT(when, Rf_install("time")))
        || check_real_arg(GET_SLOT(when, Rf_install("offset"))))
        return 1;

    return 0;
}

/**
 * Check string argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else 1
 */
int git2r_error_check_string_arg(SEXP arg)
{
    if (R_NilValue == arg
        || !isString(arg)
        || 1 != length(arg)
        || NA_STRING == STRING_ELT(arg, 0))
        return 1;
    return 0;
}

/**
 * Check tag argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else 1
 */
int git2r_error_check_tag_arg(SEXP arg)
{
    SEXP class_name;

    if (R_NilValue == arg || S4SXP != TYPEOF(arg))
        return 1;

    class_name = getAttrib(arg, R_ClassSymbol);
    if (0 != strcmp(CHAR(STRING_ELT(class_name, 0)), "git_tag"))
        return 1;

    if (git2r_error_check_string_arg(GET_SLOT(arg, Rf_install("target"))))
        return 1;

    return 0;
}

/**
 * Check tree argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else 1
 */
int git2r_error_check_tree_arg(SEXP arg)
{
    SEXP class_name;

    if (R_NilValue == arg || S4SXP != TYPEOF(arg))
        return 1;

    class_name = getAttrib(arg, R_ClassSymbol);
    if (0 != strcmp(CHAR(STRING_ELT(class_name, 0)), "git_tree"))
        return 1;

    if (git2r_error_check_string_arg(GET_SLOT(arg, Rf_install("hex"))))
        return 1;

    return 0;
}
