/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2017 The git2r contributors
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

#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>

#include "buffer.h"
#include "common.h"

#include "git2r_cred.h"
#include "git2r_transfer.h"

/**
 * Create credential object from S4 class 'cred_ssh_key'.
 *
 * @param cred The newly created credential object.
 * @param user_from_url The username that was embedded in a "user@host"
 * @param allowed_types A bitmask stating which cred types are OK to return.
 * @param credentials The S4 class object with credentials.
 * @return 0 on success, else -1.
 */
static int git2r_cred_ssh_key(
    git_cred **cred,
    const char *username_from_url,
    unsigned int allowed_types,
    SEXP credentials)
{
    if (GIT_CREDTYPE_SSH_KEY & allowed_types) {
        SEXP slot;
        const char *publickey;
        const char *privatekey = NULL;
        const char *passphrase = NULL;

        publickey = CHAR(STRING_ELT(
                             GET_SLOT(credentials,
                                      Rf_install("publickey")), 0));
        privatekey = CHAR(STRING_ELT(
                              GET_SLOT(credentials,
                                       Rf_install("privatekey")), 0));

        slot = GET_SLOT(credentials, Rf_install("passphrase"));
        if (length(slot) && (NA_STRING != STRING_ELT(slot, 0)))
            passphrase = CHAR(STRING_ELT(slot, 0));

        if (git_cred_ssh_key_new(
                cred, username_from_url, publickey, privatekey, passphrase))
            return -1;

        return 0;
    }

    return -1;
}

/**
 * Create credential object from S4 class 'cred_env'.
 *
 * @param cred The newly created credential object.
 * @param allowed_types A bitmask stating which cred types are OK to return.
 * @param credentials The S4 class object with credentials.
 * @return 0 on success, else -1.
 */
static int git2r_cred_env(
    git_cred **cred,
    unsigned int allowed_types,
    SEXP credentials)
{
    if (GIT_CREDTYPE_USERPASS_PLAINTEXT & allowed_types) {
        int err;
        git_buf username = GIT_BUF_INIT;
        git_buf password = GIT_BUF_INIT;

        /* Read value of the username environment variable */
        err = git__getenv(&username,
                          CHAR(STRING_ELT(
                                   GET_SLOT(credentials,
                                            Rf_install("username")), 0)));
        if (err)
            goto cleanup;

        if (!git_buf_len(&username)) {
            err = -1;
            goto cleanup;
        }

        /* Read value of the password environment variable */
        err = git__getenv(&password,
                          CHAR(STRING_ELT(
                                   GET_SLOT(credentials,
                                            Rf_install("password")), 0)));
        if (err)
            goto cleanup;

        if (!git_buf_len(&password)) {
            err = -1;
            goto cleanup;
        }

        err = git_cred_userpass_plaintext_new(
            cred,
            git_buf_cstr(&username),
            git_buf_cstr(&password));

    cleanup:
        git_buf_free(&username);
        git_buf_free(&password);

        if (err)
            return -1;

        return 0;
    }

    return -1;
}

/**
 * Create credential object from S4 class 'cred_token'.
 *
 * @param cred The newly created credential object.
 * @param allowed_types A bitmask stating which cred types are OK to return.
 * @param credentials The S4 class object with credentials.
 * @return 0 on success, else -1.
 */
static int git2r_cred_token(
    git_cred **cred,
    unsigned int allowed_types,
    SEXP credentials)
{
    if (GIT_CREDTYPE_USERPASS_PLAINTEXT & allowed_types) {
        int err;
        git_buf token = GIT_BUF_INIT;

        /* Read value of the personal access token from the
         * environment variable */
        err = git__getenv(&token,
                          CHAR(STRING_ELT(GET_SLOT(credentials,
                                                   Rf_install("token")), 0)));
        if (err)
            goto cleanup;

        err = git_cred_userpass_plaintext_new(cred, " ", git_buf_cstr(&token));

    cleanup:
        git_buf_free(&token);

        if (err)
            return -1;

        return 0;
    }

    return -1;
}

/**
 * Create credential object from S4 class 'cred_user_pass'.
 *
 * @param cred The newly created credential object.
 * @param allowed_types A bitmask stating which cred types are OK to return.
 * @param credentials The S4 class object with credentials.
 * @return 0 on success, else -1.
 */
static int git2r_cred_user_pass(
    git_cred **cred,
    unsigned int allowed_types,
    SEXP credentials)
{
    if (GIT_CREDTYPE_USERPASS_PLAINTEXT & allowed_types) {
        const char *username;
        const char *password;

        username = CHAR(STRING_ELT(
                            GET_SLOT(credentials,
                                     Rf_install("username")), 0));
        password = CHAR(STRING_ELT(
                            GET_SLOT(credentials,
                                     Rf_install("password")), 0));

        if (git_cred_userpass_plaintext_new(cred, username, password))
            return -1;

        return 0;
    }

    return -1;
}

/**
 * Callback if the remote host requires authentication in order to
 * connect to it
 *
 * @param cred The newly created credential object.
 * @param url The resource for which we are demanding a credential.
 * @param user_from_url The username that was embedded in a "user@host"
 * remote url, or NULL if not included.
 * @param allowed_types A bitmask stating which cred types are OK to return.
 * @param payload The payload provided when specifying this callback.
 * @return 0 on success, else -1.
 */
int git2r_cred_acquire_cb(
    git_cred **cred,
    const char *url,
    const char *username_from_url,
    unsigned int allowed_types,
    void *payload)
{
    SEXP credentials, class_name;

    GIT_UNUSED(url);

    if (!payload)
        return -1;

    credentials = ((git2r_transfer_data*)payload)->credentials;
    if (Rf_isNull(credentials)) {
        if (GIT_CREDTYPE_SSH_KEY & allowed_types) {
	    if (((git2r_transfer_data*)payload)->ssh_key_agent_tried)
	        return -1;
	    ((git2r_transfer_data*)payload)->ssh_key_agent_tried = 1;
            if (git_cred_ssh_key_from_agent(cred, username_from_url))
                return -1;
            return 0;
        }

        return -1;
    }

    class_name = getAttrib(credentials, R_ClassSymbol);
    if (strcmp(CHAR(STRING_ELT(class_name, 0)), "cred_ssh_key") == 0) {
        return git2r_cred_ssh_key(
            cred, username_from_url, allowed_types, credentials);
    } else if (strcmp(CHAR(STRING_ELT(class_name, 0)), "cred_env") == 0) {
        return git2r_cred_env(cred, allowed_types, credentials);
    } else if (strcmp(CHAR(STRING_ELT(class_name, 0)), "cred_token") == 0) {
        return git2r_cred_token(cred, allowed_types, credentials);
    } else if (strcmp(CHAR(STRING_ELT(class_name, 0)), "cred_user_pass") == 0) {
        return git2r_cred_user_pass(cred, allowed_types, credentials);
    }

    return -1;
}
