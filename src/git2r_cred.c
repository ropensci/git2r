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

#ifdef WIN32
#include <windows.h>
#include <wchar.h>

# ifndef WC_ERR_INVALID_CHARS
#  define WC_ERR_INVALID_CHARS	0x80
# endif
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include <git2.h>

#include "git2r_cred.h"
#include "git2r_S3.h"
#include "git2r_transfer.h"

/**
 * Read an environtmental variable.
 *
 * @param out Pointer where to store the environmental variable.
 * @param obj The S3 object with name of the environmental
 *   variable to read.
 * @param slot The slot in the S3 object with the name of the
 *   environmental variable.
 * @return 0 on success, else -1.
 */
static int git2r_getenv(char **out, SEXP obj, const char *slot)
{
    const char *buf;

    /* Read value of the environment variable */
    buf = getenv(CHAR(STRING_ELT(git2r_get_list_element(obj, slot), 0)));
    if (!buf || !strlen(buf))
        return -1;

    *out = malloc(strlen(buf)+1);
    if (!*out)
        return -1;

    strcpy(*out, buf);

    return 0;
}

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
        char *username = NULL;
        char *password = NULL;

        /* Read value of the username environment variable */
        error = git2r_getenv(&username, credentials, "username");
        if (error)
            goto cleanup;

        /* Read value of the password environment variable */
        error = git2r_getenv(&password, credentials, "password");
        if (error)
            goto cleanup;

        error = git_cred_userpass_plaintext_new(cred, username, password);

    cleanup:
        free(username);
        free(password);

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
        char *token = NULL;

        /* Read value of the personal access token from the
         * environment variable */
        error = git2r_getenv(&token, credentials, "token");
        if (error)
            goto cleanup;

        error = git_cred_userpass_plaintext_new(cred, " ", token);

    cleanup:
        free(token);

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

static int git2r_file_exists(const char *path)
{
#ifdef WIN32
    struct _stati64 sb;
    return _stati64(path, &sb) == 0;
#else
    struct stat sb;
    return stat(path, &sb) == 0;
#endif
}

#ifdef WIN32
int git2r_path_from_environment_variable(char** out, wchar_t *env)
{
    wchar_t path[MAX_PATH];
    DWORD len;
    int len_utf8;

    /* Expands environment-variable strings and replaces them with the
     * values defined for the current user. */
    len = ExpandEnvironmentStringsW(env, path, MAX_PATH);
    if (!len || len > MAX_PATH)
        return -1;

    /* Map wide character string to a new utf8 character string. */
    len_utf8 = WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS, path,-1, NULL, 0, NULL, NULL);
    if (!len_utf8)
        return -1;

    *out = malloc(len_utf8);
    if (!*out)
        return -1;

    len_utf8 = WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS, path, -1, *out, len_utf8, NULL, NULL);
    if (!len_utf8) {
        free(*out);
        *out = NULL;
        return -1;
    }

    return 0;
}
#endif

SEXP git2r_ssh_keys()
{
    const char *keys [] = {"id_ed25519", "id_ecdsa", "id_rsa", "id_dsa", NULL};

#ifdef WIN32
    const wchar_t *env =
        {L"%HOME%\\", L"%HOMEDRIVE%%HOMEPATH%\\", L"%USERPROFILE%\\", NULL};
    int i;

    for (i = 0; env[i]; i++) {
        char *path = NULL;
        int j;

        if (git2r_path_from_environment_variable(&path, env[i]))
            continue;

        for (j = 0; keys[j]; j++) {
            char *private_key = NULL;
            int private_key_len;
            int n;

            private_key_len = strlen(path) + sizeof(".ssh/") + strlen(keys[j]);
            private_key = malloc(private_key_len);
            if (!private_key)
                continue;
            n = snprintf(private_key, private_key_len, "%s.ssh/%s", path, keys[j]);
            if (n < 0 || n >= private_key_len) {
                free(private_key);
                continue;
            }

            if (git2r_file_exists(private_key)) {
                int public_key_len = strlen(private_key) + sizeof(".pub");
                char *public_key = malloc(public_key_len);
                if (!public_key) {
                    free(private_key);
                    continue;
                }
                n = snprintf(public_key, public_key_len, "%s.pub", private_key);
                if (n < 0 || n >= public_key_len) {
                    free(private_key);
                    free(public_key);
                    continue;
                }

                if (git2r_file_exists(private_key))
                    Rprintf("private: %s public: %s\n", private_key, public_key);
                free(public_key);
            }

            free(private_key);
        }

        free(path);
    }
#else
    const char *path = getenv("HOME");
    Rprintf("path: %s\n", path);
#endif

    /* const char *home [] = {R_ExpandFileName("~"), NULL}; */
    /* const char *keys [] = {"id_ed25519", "id_ecdsa", "id_rsa", "id_dsa", NULL}; */
    /* int i; */

    /* for (i = 0; home[i]; i++) { */
    /*     int j; */

    /*     for (j = 0; keys[j]; j++) { */
    /*         int n; */
    /*         int private_key_len = strlen(home[i]) + sizeof("/.ssh/") + strlen(keys[j]); */
    /*         char *private_key = malloc(private_key_len); */
    /*         if (!private_key) */
    /*             Rf_error("FIXME"); */
    /*         n = snprintf(private_key, private_key_len, "%s/.ssh/%s", home[i], keys[j]); */
    /*         if (n < 0 || n >= private_key_len) */
    /*             Rf_error("FIXME"); */

    /*         if (git2r_file_exists(private_key)) { */
    /*             int public_key_len = strlen(private_key) + sizeof(".pub"); */
    /*             char *public_key = malloc(public_key_len); */
    /*             if (!private_key) */
    /*                 Rf_error("FIXME"); */
    /*             n = snprintf(public_key, public_key_len, "%s.pub", private_key); */
    /*             if (n < 0 || n >= public_key_len) */
    /*                 Rf_error("FIXME"); */

    /*             Rprintf("private: %s public: %s\n", private_key, public_key); */

    /*             free(public_key); */
    /*         } */

    /*         free(private_key); */
    /*     } */
    /* } */

    return R_NilValue;
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
