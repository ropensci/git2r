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
#include "kvec.h"

typedef struct git2r_ssh_key
{
    char *private;
    char *public;
} git2r_ssh_key;

typedef kvec_t(git2r_ssh_key) git2r_ssh_key_t;

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

#ifdef WIN32
static int git2r_expand_path(char** out, const wchar_t *path)
{
    struct _stati64 sb;
    wchar_t buf[MAX_PATH];
    DWORD len;
    int len_utf8;

    *out = NULL;

    /* Expands environment-variable strings and replaces them with the
     * values defined for the current user. */
    len = ExpandEnvironmentStringsW(path, buf, MAX_PATH);
    if (!len || len > MAX_PATH)
        goto on_error;

    /* Map wide character string to a new utf8 character string. */
    len_utf8 = WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS, buf,-1, NULL, 0, NULL, NULL);
    if (!len_utf8)
        goto on_error;

    *out = malloc(len_utf8);
    if (!*out)
        goto on_error;

    len_utf8 = WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS, buf, -1, *out, len_utf8, NULL, NULL);
    if (!len_utf8)
        goto on_error;

    /* Check if file exists. */
    if (_stati64(*out, &sb) == 0)
        return 0;

on_error:
    free(*out);
    *out = NULL;

    return -1;
}
#else
static int git2r_expand_path(char** out, const char *path)
{
    struct stat sb;
    int len;
    const char *buf = R_ExpandFileName(path);

    *out = NULL;
    if (!buf)
        goto on_error;
    len = strlen(buf);
    if (len <= 0)
        goto on_error;
    *out = malloc(len + 1);
    if (!*out)
        goto on_error;
    strncpy(*out, buf, len);
    (*out)[len] = '\0';

    /* Check if file exists. */
    if (stat(*out, &sb) == 0)
        return 0;

on_error:
    free(*out);
    *out = NULL;

    return -1;
}
#endif

static void git2r_default_ssh_keys(git2r_ssh_key_t *keys)
{
#ifdef WIN32
    static const wchar_t *private_key_patterns[13] =
        {L"%HOME%\\.ssh\\id_ed25519",
         L"%HOME%\\.ssh\\id_ecdsa",
         L"%HOME%\\.ssh\\id_rsa",
         L"%HOME%\\.ssh\\id_dsa",
         L"%HOMEDRIVE%%HOMEPATH%\\.ssh\\id_ed25519",
         L"%HOMEDRIVE%%HOMEPATH%\\.ssh\\id_ecdsa",
         L"%HOMEDRIVE%%HOMEPATH%\\.ssh\\id_rsa",
         L"%HOMEDRIVE%%HOMEPATH%\\.ssh\\id_dsa",
         L"%USERPROFILE%\\.ssh\\id_ed25519",
         L"%USERPROFILE%\\.ssh\\id_ecdsa",
         L"%USERPROFILE%\\.ssh\\id_rsa",
         L"%USERPROFILE%\\.ssh\\id_dsa",
         NULL};

    static const wchar_t *public_key_patterns[13] =
        {L"%HOME%\\.ssh\\id_ed25519.pub",
         L"%HOME%\\.ssh\\id_ecdsa.pub",
         L"%HOME%\\.ssh\\id_rsa.pub",
         L"%HOME%\\.ssh\\id_dsa.pub",
         L"%HOMEDRIVE%%HOMEPATH%\\.ssh\\id_ed25519.pub",
         L"%HOMEDRIVE%%HOMEPATH%\\.ssh\\id_ecdsa.pub",
         L"%HOMEDRIVE%%HOMEPATH%\\.ssh\\id_rsa.pub",
         L"%HOMEDRIVE%%HOMEPATH%\\.ssh\\id_dsa.pub",
         L"%USERPROFILE%\\.ssh\\id_ed25519.pub",
         L"%USERPROFILE%\\.ssh\\id_ecdsa.pub",
         L"%USERPROFILE%\\.ssh\\id_rsa.pub",
         L"%USERPROFILE%\\.ssh\\id_dsa.pub",
         NULL};
#else
    static const char *private_key_patterns[5] =
        {"~/.ssh/id_ed25519",
         "~/.ssh/id_ecdsa",
         "~/.ssh/id_rsa",
         "~/.ssh/id_dsa",
         NULL};

    static const char *public_key_patterns[5] =
        {"~/.ssh/id_ed25519.pub",
         "~/.ssh/id_ecdsa.pub",
         "~/.ssh/id_rsa.pub",
         "~/.ssh/id_dsa.pub",
         NULL};
#endif
    int i;

    /* Find unique keys. */
    for (i = 0; private_key_patterns[i] && public_key_patterns[i]; i++) {
        char *private_key = NULL;
        char *public_key = NULL;
        int duplicate_key = 0;
        int j;

        if (git2r_expand_path(&private_key, private_key_patterns[i]) ||
            git2r_expand_path(&public_key, public_key_patterns[i]))
        {
            free(private_key);
            free(public_key);
            continue;
        }

        for (j = 0; j < kv_size(*keys); j++) {
            if (strcmp(private_key, kv_A(*keys, j).private) == 0 ||
                strcmp(public_key, kv_A(*keys, j).public) == 0) {
                duplicate_key = 1;
                break;
            }
        }

        if (duplicate_key) {
            free(private_key);
            free(public_key);
        } else {
            const git2r_ssh_key key = {private_key, public_key};
            kv_push(git2r_ssh_key, *keys, key);
        }
    }

}

SEXP git2r_ssh_keys()
{
    git2r_ssh_key_t keys;
    int i;

    kv_init(keys);
    git2r_default_ssh_keys(&keys);

    /* Print keys. */
    for (i = 0; i < kv_size(keys); i++)
        Rprintf("private: %s, public: %s\n", kv_A(keys, i).private, kv_A(keys, i).public);

    for (i = 0; i < kv_size(keys); i++) {
        free(kv_A(keys, i).private);
        free(kv_A(keys, i).public);
    }

    kv_destroy(keys);

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
