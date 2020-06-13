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
#include "git2r_deprecated.h"
#include "git2r_error.h"
#include "git2r_S3.h"

/**
 * Check blob argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else -1
 */
int attribute_hidden
git2r_arg_check_blob(
    SEXP arg)
{
    if (!Rf_isNewList(arg) || !Rf_inherits(arg, "git_blob"))
        return -1;

    if (git2r_arg_check_sha(git2r_get_list_element(arg, "sha")))
        return -1;

    return 0;
}

/**
 * Check branch argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else -1
 */
int attribute_hidden
git2r_arg_check_branch(
    SEXP arg)
{
    SEXP slot;

    if (!Rf_isNewList(arg) || !Rf_inherits(arg, "git_branch"))
        return -1;

    if (git2r_arg_check_string(git2r_get_list_element(arg, "name")))
        return -1;

    slot = git2r_get_list_element(arg, "type");
    if (git2r_arg_check_integer(slot))
        return -1;
    switch (INTEGER(slot)[0]) {
    case GIT_BRANCH_LOCAL:
    case GIT_BRANCH_REMOTE:
        break;
    default:
        return -1;
    }

    return 0;
}

/**
 * Check commit argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else -1
 */
int attribute_hidden
git2r_arg_check_commit(
    SEXP arg)
{
    if (!Rf_isNewList(arg) || !Rf_inherits(arg, "git_commit"))
        return -1;

    if (git2r_arg_check_sha(git2r_get_list_element(arg, "sha")))
        return -1;

    return 0;
}

/**
 * Check commit or stash argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else -1
 */
int attribute_hidden
git2r_arg_check_commit_stash(
    SEXP arg)
{
    if (!Rf_isNewList(arg))
        return -1;

    if (!Rf_inherits(arg, "git_commit") && !Rf_inherits(arg, "git_stash"))
        return -1;

    if (git2r_arg_check_sha(git2r_get_list_element(arg, "sha")))
        return -1;

    return 0;
}

/**
 * Check credentials argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else -1
 */
int attribute_hidden
git2r_arg_check_credentials(
    SEXP arg)
{
    /* It's ok if the credentials is R_NilValue */
    if (Rf_isNull(arg))
        return 0;

    if (!Rf_isNewList(arg))
        return -1;

    if (Rf_inherits(arg, "cred_env")) {
        /* Check username and password */
        if (git2r_arg_check_string(git2r_get_list_element(arg, "username")))
            return -1;
        if (git2r_arg_check_string(git2r_get_list_element(arg, "password")))
            return -1;
    } else if (Rf_inherits(arg, "cred_token")) {
        /* Check token */
        if (git2r_arg_check_string(git2r_get_list_element(arg, "token")))
            return -1;
    } else if (Rf_inherits(arg, "cred_user_pass")) {
        /* Check username and password */
        if (git2r_arg_check_string(git2r_get_list_element(arg, "username")))
            return -1;
        if (git2r_arg_check_string(git2r_get_list_element(arg, "password")))
            return -1;
    } else if (Rf_inherits(arg, "cred_ssh_key")) {
        SEXP passphrase;

        /* Check public and private key */
        if (git2r_arg_check_string(git2r_get_list_element(arg, "publickey")))
            return -1;
        if (git2r_arg_check_string(git2r_get_list_element(arg, "privatekey")))
            return -1;

        /* Check that passphrase is a character vector */
        passphrase = git2r_get_list_element(arg, "passphrase");
        if (git2r_arg_check_string_vec(passphrase))
            return -1;

        /* Check that length of passphrase < 2, i.e. it's either
         * character(0) or some "passphrase" */
        switch (Rf_length(passphrase)) {
        case 0:
            break;
        case 1:
            if (NA_STRING == STRING_ELT(passphrase, 0))
                return -1;
            break;
        default:
            return -1;
        }
    } else {
        return -1;
    }

    return 0;
}

/**
 * Check fetch_heads argument
 *
 * It's OK:
 *  - A list with S3 class git_fetch_head objects
 * @param arg the arg to check
 * @return 0 if OK, else -1
 */
int attribute_hidden
git2r_arg_check_fetch_heads(
    SEXP arg)
{
    const char *repo = NULL;
    size_t i,n;

    if (Rf_isNull(arg) || VECSXP != TYPEOF(arg))
        return -1;

    /* Check that the repository paths are identical for each item */
    n = Rf_length(arg);
    for (i = 0; i < n; i++) {
        SEXP path;
        SEXP item = VECTOR_ELT(arg, i);

        if (!Rf_isNewList(item) || !Rf_inherits(item, "git_fetch_head"))
            return -1;

        path = git2r_get_list_element(git2r_get_list_element(item, "repo"), "path");
        if (git2r_arg_check_string(path))
            return -1;

        if (0 == i)
            repo = CHAR(STRING_ELT(path, 0));
        else if (0 != strcmp(repo, CHAR(STRING_ELT(path, 0))))
            return -1;
    }

    return 0;
}

/**
 * Check filename argument
 *
 * It's OK:
 *  - R_NilValue
 *  - Zero length character vector
 *  - character vector of length one with strlen(value) > 0
 * @param arg the arg to check
 * @return 0 if OK, else -1
 */
int attribute_hidden
git2r_arg_check_filename(
    SEXP arg)
{
    if (Rf_isNull(arg))
        return 0;
    if (!Rf_isString(arg))
        return -1;
    switch (Rf_length(arg)) {
    case 0:
        break;
    case 1:
        if (NA_STRING == STRING_ELT(arg, 0))
            return -1;
        if (0 == strlen(CHAR(STRING_ELT(arg, 0))))
            return -1;
        break;
    default:
        return -1;
    }

    return 0;
}

/**
 * Check sha argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else -1
 */
int attribute_hidden
git2r_arg_check_sha(
    SEXP arg)
{
    size_t len;

    if (git2r_arg_check_string(arg))
        return -1;

    len = LENGTH(STRING_ELT(arg, 0));
    if (len < GIT_OID_MINPREFIXLEN || len > GIT_OID_HEXSZ)
        return -1;

    return 0;
}

/**
 * Check integer argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else -1
 */
int attribute_hidden
git2r_arg_check_integer(
    SEXP arg)
{
    if (!Rf_isInteger(arg) || 1 != Rf_length(arg) || NA_INTEGER == INTEGER(arg)[0])
        return -1;
    return 0;
}

/**
 * Check integer argument and that arg is greater than or equal to 0.
 *
 * @param arg the arg to check
 * @return 0 if OK, else -1
 */
int attribute_hidden
git2r_arg_check_integer_gte_zero(
    SEXP arg)
{
    if (git2r_arg_check_integer(arg))
        return -1;
    if (0 > INTEGER(arg)[0])
        return -1;
    return 0;
}

/**
 * Check list argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else -1
 */
int attribute_hidden
git2r_arg_check_list(
    SEXP arg)
{
    if (!Rf_isNewList(arg))
        return -1;
    return 0;
}


/**
 * Check logical argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else -1
 */
int attribute_hidden
git2r_arg_check_logical(
    SEXP arg)
{
    if (!Rf_isLogical(arg) || 1 != Rf_length(arg) || NA_LOGICAL == LOGICAL(arg)[0])
        return -1;
    return 0;
}

/**
 * Check note argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else -1
 */
int attribute_hidden
git2r_arg_check_note(
    SEXP arg)
{
    if (!Rf_isNewList(arg) || !Rf_inherits(arg, "git_note"))
        return -1;

    if (git2r_arg_check_sha(git2r_get_list_element(arg, "sha")))
        return -1;

    if (git2r_arg_check_string(git2r_get_list_element(arg, "refname")))
        return -1;

    return 0;
}

/**
 * Check real argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else -1
 */
int attribute_hidden
git2r_arg_check_real(
    SEXP arg)
{
    if (!Rf_isReal(arg) || 1 != Rf_length(arg) || !R_finite(REAL(arg)[0]))
        return -1;
    return 0;
}

/**
 * Check repository argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else -1
 */
int attribute_hidden
git2r_arg_check_repository(
    SEXP arg)
{
    if (!Rf_isNewList(arg) || !Rf_inherits(arg, "git_repository"))
        return -1;

    if (git2r_arg_check_string(git2r_get_list_element(arg, "path")))
        return -1;

    return 0;
}

/**
 * Check if the two repositories have the same path
 *
 * @param arg the arg to check
 * @return 0 if OK, else -1
 */
int attribute_hidden
git2r_arg_check_same_repo(
    SEXP arg1,
    SEXP arg2)
{
    SEXP path1, path2;

    if (git2r_arg_check_repository(arg1) || git2r_arg_check_repository(arg2))
        return -1;

    path1 = git2r_get_list_element(arg1, "path");
    path2 = git2r_get_list_element(arg2, "path");
    if (strcmp(CHAR(STRING_ELT(path1, 0)), CHAR(STRING_ELT(path2, 0))))
        return -1;

    return 0;
}

/**
 * Check signature argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else -1
 */
int attribute_hidden
git2r_arg_check_signature(
    SEXP arg)
{
    SEXP when;

    if (!Rf_isNewList(arg) || !Rf_inherits(arg, "git_signature"))
        return -1;

    if (git2r_arg_check_string(git2r_get_list_element(arg, "name")))
        return -1;
    if (git2r_arg_check_string(git2r_get_list_element(arg, "email")))
        return -1;

    when = git2r_get_list_element(arg, "when");
    if (git2r_arg_check_real(git2r_get_list_element(when, "time")))
        return -1;
    if (git2r_arg_check_real(git2r_get_list_element(when, "offset")))
        return -1;

    return 0;
}

/**
 * Check string argument
 *
 * Compared to git2r_arg_check_string_vec, also checks that length of vector
 * is one and non-NA.
 * @param arg the arg to check
 * @return 0 if OK, else -1
 */
int attribute_hidden
git2r_arg_check_string(
    SEXP arg)
{
    if (git2r_arg_check_string_vec(arg) < 0)
        return -1;
    if (1 != Rf_length(arg) || NA_STRING == STRING_ELT(arg, 0))
        return -1;
    return 0;
}

/**
 * Check string vector argument
 *
 * Compared to git2r_arg_check_string, only checks that argument is non-null
 * and string. Use git2r_arg_check_string to check scalar string.
 * @param arg the arg to check
 * @return 0 if OK, else -1
 */
int attribute_hidden
git2r_arg_check_string_vec(
    SEXP arg)
{
    if (!Rf_isString(arg))
        return -1;
    return 0;
}

/**
 * Copy SEXP character vector to git_strarray
 *
 * @param dst Destination of data
 * @src src Source of data. Skips NA strings.
 * @return 0 if OK, else error code
 */

int attribute_hidden
git2r_copy_string_vec(
    git_strarray *dst,
    SEXP src)
{
    size_t i, len;

    /* Count number of non NA values */
    len = Rf_length(src);
    for (i = 0; i < len; i++)
        if (NA_STRING != STRING_ELT(src, i))
            dst->count++;

    /* We are done if no non-NA values  */
    if (!dst->count)
        return 0;

    /* Allocate the strings in dst */
    dst->strings = malloc(dst->count * sizeof(char*));
    if (!dst->strings) {
        GIT2R_ERROR_SET_STR(GIT2R_ERROR_NONE, git2r_err_alloc_memory_buffer);
        return GIT_ERROR;
    }

    /* Copy strings to dst */
    for (i = 0; i < dst->count; i++)
        if (NA_STRING != STRING_ELT(src, i))
            dst->strings[i] = (char *)CHAR(STRING_ELT(src, i));

    return 0;
}

/**
 * Check tag argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else -1
 */
int attribute_hidden
git2r_arg_check_tag(
    SEXP arg)
{
    if (!Rf_isNewList(arg) || !Rf_inherits(arg, "git_tag"))
        return -1;

    if (git2r_arg_check_string(git2r_get_list_element(arg, "target")))
        return -1;

    return 0;
}

/**
 * Check tree argument
 *
 * @param arg the arg to check
 * @return 0 if OK, else -1
 */
int attribute_hidden
git2r_arg_check_tree(
    SEXP arg)
{
    if (!Rf_isNewList(arg) || !Rf_inherits(arg, "git_tree"))
        return -1;

    if (git2r_arg_check_sha(git2r_get_list_element(arg, "sha")))
        return -1;

    return 0;
}
