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

#include <R.h>
#include <Rinternals.h>

#include <git2.h>
#include "buffer.h"

#include "git2r_cred.h"
#include "git2r_S3.h"
#include "git2r_transfer.h"

/**
 * Create credential object from S3 class 'cred_ssh_key'.
 *
 * @param cred The newly created credential object.
 * @param user_from_url The username that was embedded in a "user@host"
 * @param allowed_types A bitmask stating which cred types are OK to return.
 * @param credentials The S3 class object with credentials.
 * @return 0 on success, else -1.
 */
static int git2r_cred_ssh_key(
    git_cred **cred,
    const char *username_from_url,
    unsigned int allowed_types,
    SEXP credentials)
{
    if (GIT_CREDTYPE_SSH_KEY & allowed_types) {
        SEXP elem;
        const char *publickey;
        const char *privatekey = NULL;
        const char *passphrase = NULL;

        publickey = CHAR(STRING_ELT(git2r_get_list_element(credentials, "publickey"), 0));
        privatekey = CHAR(STRING_ELT(git2r_get_list_element(credentials, "privatekey"), 0));

        elem = git2r_get_list_element(credentials, "passphrase");
        if (Rf_length(elem) && (NA_STRING != STRING_ELT(elem, 0)))
            passphrase = CHAR(STRING_ELT(elem, 0));

        if (git_cred_ssh_key_new(
                cred, username_from_url, publickey, privatekey, passphrase))
            return -1;

        return 0;
    }

    return -1;
}

/**
 * Create credential object from S3 class 'cred_env'.
 *
 * @param cred The newly created credential object.
 * @param allowed_types A bitmask stating which cred types are OK to return.
 * @param credentials The S3 class object with credentials.
 * @return 0 on success, else -1.
 */
static int git2r_cred_env(
    git_cred **cred,
    unsigned int allowed_types,
    SEXP credentials)
{
    if (GIT_CREDTYPE_USERPASS_PLAINTEXT & allowed_types) {
        int error;
        git_buf username = GIT_BUF_INIT;
        git_buf password = GIT_BUF_INIT;

        /* Read value of the username environment variable */
        error = git__getenv(
            &username,
            CHAR(STRING_ELT(git2r_get_list_element(credentials, "username"), 0)));
        if (error)
            goto cleanup;

        if (!git_buf_len(&username)) {
            error = -1;
            goto cleanup;
        }

        /* Read value of the password environment variable */
        error = git__getenv(
            &password,
            CHAR(STRING_ELT(git2r_get_list_element(credentials, "password"), 0)));
        if (error)
            goto cleanup;

        if (!git_buf_len(&password)) {
            error = -1;
            goto cleanup;
        }

        error = git_cred_userpass_plaintext_new(
            cred,
            username.ptr,
            password.ptr);

    cleanup:
        git_buf_free(&username);
        git_buf_free(&password);

        if (error)
            return -1;

        return 0;
    }

    return -1;
}

/**
 * Create credential object from S3 class 'cred_token'.
 *
 * @param cred The newly created credential object.
 * @param allowed_types A bitmask stating which cred types are OK to return.
 * @param credentials The S3 class object with credentials.
 * @return 0 on success, else -1.
 */
static int git2r_cred_token(
    git_cred **cred,
    unsigned int allowed_types,
    SEXP credentials)
{
    if (GIT_CREDTYPE_USERPASS_PLAINTEXT & allowed_types) {
        int error;
        git_buf token = GIT_BUF_INIT;

        /* Read value of the personal access token from the
         * environment variable */
        error = git__getenv(
            &token,
            CHAR(STRING_ELT(git2r_get_list_element(credentials, "token"), 0)));
        if (error)
            goto cleanup;

        error = git_cred_userpass_plaintext_new(cred, " ", token.ptr);

    cleanup:
        git_buf_free(&token);

        if (error)
            return -1;

        return 0;
    }

    return -1;
}

/**
 * Create credential object from S3 class 'cred_user_pass'.
 *
 * @param cred The newly created credential object.
 * @param allowed_types A bitmask stating which cred types are OK to return.
 * @param credentials The S3 class object with credentials.
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

        username = CHAR(STRING_ELT(git2r_get_list_element(credentials, "username"), 0));
        password = CHAR(STRING_ELT(git2r_get_list_element(credentials, "password"), 0));
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
    SEXP credentials;

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

    if (Rf_inherits(credentials, "cred_ssh_key")) {
        return git2r_cred_ssh_key(
            cred, username_from_url, allowed_types, credentials);
    } else if (Rf_inherits(credentials, "cred_env")) {
        return git2r_cred_env(cred, allowed_types, credentials);
    } else if (Rf_inherits(credentials, "cred_token")) {
        return git2r_cred_token(cred, allowed_types, credentials);
    } else if (Rf_inherits(credentials, "cred_user_pass")) {
        return git2r_cred_user_pass(cred, allowed_types, credentials);
    }

    return -1;
}
