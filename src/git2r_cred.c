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

#include "git2r_arg.h"
#include "git2r_cred.h"
#include "git2r_deprecated.h"
#include "git2r_S3.h"
#include "git2r_transfer.h"

#define GIT2R_ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

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
static int
git2r_getenv(
    char **out, SEXP obj,
    const char *slot)
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
static int
git2r_cred_ssh_key(
    GIT2R_CREDENTIAL **cred,
    const char *username_from_url,
    unsigned int allowed_types,
    SEXP credentials)
{
    if (GIT2R_CREDENTIAL_SSH_KEY & allowed_types) {
        SEXP elem;
        const char *publickey;
        const char *privatekey = NULL;
        const char *passphrase = NULL;

        publickey = CHAR(STRING_ELT(git2r_get_list_element(credentials, "publickey"), 0));
        privatekey = CHAR(STRING_ELT(git2r_get_list_element(credentials, "privatekey"), 0));

        elem = git2r_get_list_element(credentials, "passphrase");
        if (Rf_length(elem) && (NA_STRING != STRING_ELT(elem, 0)))
            passphrase = CHAR(STRING_ELT(elem, 0));

        if (GIT2R_CREDENTIAL_SSH_KEY_NEW(
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
static int
git2r_cred_env(
    GIT2R_CREDENTIAL **cred,
    unsigned int allowed_types,
    SEXP credentials)
{
    if (GIT2R_CREDENTIAL_USERPASS_PLAINTEXT & allowed_types) {
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

        error = GIT2R_CREDENTIAL_USERPASS_PLAINTEXT_NEW(
            cred, username, password);

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
static int
git2r_cred_token(
    GIT2R_CREDENTIAL **cred,
    unsigned int allowed_types,
    SEXP credentials)
{
    if (GIT2R_CREDENTIAL_USERPASS_PLAINTEXT & allowed_types) {
        int error;
        char *token = NULL;

        /* Read value of the personal access token from the
         * environment variable */
        error = git2r_getenv(&token, credentials, "token");
        if (error)
            goto cleanup;

        error = GIT2R_CREDENTIAL_USERPASS_PLAINTEXT_NEW(cred, " ", token);

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
static int
git2r_cred_user_pass(
    GIT2R_CREDENTIAL **cred,
    unsigned int allowed_types,
    SEXP credentials)
{
    if (GIT2R_CREDENTIAL_USERPASS_PLAINTEXT & allowed_types) {
        const char *username;
        const char *password;

        username = CHAR(STRING_ELT(git2r_get_list_element(credentials, "username"), 0));
        password = CHAR(STRING_ELT(git2r_get_list_element(credentials, "password"), 0));
        if (GIT2R_CREDENTIAL_USERPASS_PLAINTEXT_NEW(cred, username, password))
            return -1;

        return 0;
    }

    return -1;
}

/* static int git2r_join_str(char** out, const char *str_a, const char *str_b) */
/* { */
/*     int len_a, len_b; */

/*     if (!str_a || !str_b) */
/*         return -1; */

/*     len_a = strlen(str_a); */
/*     len_b = strlen(str_b); */

/*     *out = malloc(len_a + len_b + 1); */
/*     if (!*out) */
/*         return -1; */

/*     if (len_a) */
/*         memcpy(*out, str_a, len_a); */
/*     if (len_b) */
/*         memcpy(*out + len_a, str_b, len_b); */
/*     (*out)[len_a + len_b] = '\0'; */

/*     return 0; */
/* } */

/* static int git2r_file_exists(const char *path) */
/* { */
/* #ifdef WIN32 */
/*     struct _stati64 sb; */
/*     return _stati64(path, &sb) == 0; */
/* #else */
/*     struct stat sb; */
/*     return stat(path, &sb) == 0; */
/* #endif */
/* } */

/* #ifdef WIN32 */
/* static int git2r_expand_key(char** out, const wchar_t *key, const char *ext) */
/* { */
/*     wchar_t wbuf[MAX_PATH]; */
/*     char *buf_utf8 = NULL; */
/*     DWORD len_wbuf; */
/*     int len_utf8; */

/*     *out = NULL; */

/*     if (!key || !ext) */
/*         goto on_error; */

/*     /\* Expands environment-variable strings and replaces them with the */
/*      * values defined for the current user. *\/ */
/*     len_wbuf = ExpandEnvironmentStringsW(key, wbuf, GIT2R_ARRAY_SIZE(wbuf)); */
/*     if (!len_wbuf || len_wbuf > GIT2R_ARRAY_SIZE(wbuf)) */
/*         goto on_error; */

/*     /\* Map wide character string to a new utf8 character string. *\/ */
/*     len_utf8 = WideCharToMultiByte( */
/*         CP_UTF8, WC_ERR_INVALID_CHARS, wbuf,-1, NULL, 0, NULL, NULL); */
/*     if (!len_utf8) */
/*         goto on_error; */

/*     buf_utf8 = malloc(len_utf8); */
/*     if (!buf_utf8) */
/*         goto on_error; */

/*     len_utf8 = WideCharToMultiByte( */
/*         CP_UTF8, WC_ERR_INVALID_CHARS, wbuf, -1, buf_utf8, len_utf8, NULL, NULL); */
/*     if (!len_utf8) */
/*         goto on_error; */

/*     if (git2r_join_str(out, buf_utf8, ext)) */
/*         goto on_error; */
/*     free(buf_utf8); */

/*     if (git2r_file_exists(*out)) */
/*         return 0; */

/* on_error: */
/*     free(buf_utf8); */
/*     free(*out); */
/*     *out = NULL; */

/*     return -1; */
/* } */
/* #else */
/* static int git2r_expand_key(char** out, const char *key, const char *ext) */
/* { */
/*     const char *buf = R_ExpandFileName(key); */

/*     *out = NULL; */

/*     if (!key || !ext) */
/*         return -1; */

/*     if (git2r_join_str(out, buf, ext)) */
/*         return -1; */

/*     if (git2r_file_exists(*out)) */
/*         return 0; */

/*     free(*out); */
/*     *out = NULL; */

/*     return -1; */
/* } */
/* #endif */

/* static int git2r_ssh_key_needs_passphrase(const char *key) */
/* { */
/*     size_t i; */
/*     FILE* file = fopen(key, "r"); */

/*     if (file == NULL) */
/*         return 0; */

/*     /\* Look for "ENCRYPTED" in the first three lines. *\/ */
/*     for (i = 0; i < 3; i++) { */
/*         char str[128] = {0}; */
/*         if (fgets(str, GIT2R_ARRAY_SIZE(str), file) != NULL) { */
/*             if (strstr(str, "ENCRYPTED") != NULL) { */
/*                 fclose(file); */
/*                 return 1; */
/*             } */
/*         } else { */
/*             fclose(file); */
/*             return 0; */
/*         } */
/*     } */

/*     fclose(file); */

/*     return 0; */
/* } */

/* static int git2r_cred_default_ssh_key( */
/*     GIT2R_CREDENTIAL **cred, */
/*     const char *username_from_url) */
/* { */
/* #ifdef WIN32 */
/*     static const wchar_t *key_patterns[3] = */
/*         {L"%HOME%\\.ssh\\id_rsa", */
/*          L"%HOMEDRIVE%%HOMEPATH%\\.ssh\\id_rsa", */
/*          L"%USERPROFILE%\\.ssh\\id_rsa"}; */
/* #else */
/*     static const char *key_patterns[1] = {"~/.ssh/id_rsa"}; */
/* #endif */
/*     size_t i; */
/*     int error = 1; */

/*     /\* Find key. *\/ */
/*     for (i = 0; i < GIT2R_ARRAY_SIZE(key_patterns); i++) { */
/*         char *private_key = NULL; */
/*         char *public_key = NULL; */
/*         const char *passphrase = NULL; */
/*         SEXP pass, askpass, call; */
/*         int nprotect = 0; */

/*         /\* Expand key pattern and check if files exists. *\/ */
/*         if (git2r_expand_key(&private_key, key_patterns[i], "") || */
/*             git2r_expand_key(&public_key, key_patterns[i], ".pub")) */
/*         { */
/*             free(private_key); */
/*             free(public_key); */
/*             continue; */
/*         } */

/*         if (git2r_ssh_key_needs_passphrase(private_key)) { */
/*             /\* Use the R package getPass to ask for the passphrase. *\/ */
/*             PROTECT(pass = Rf_eval(Rf_lang2(Rf_install("getNamespace"), */
/*                                             Rf_ScalarString(Rf_mkChar("getPass"))), */
/*                                    R_GlobalEnv)); */
/*             nprotect++; */

/*             PROTECT(call = Rf_lcons( */
/*                         Rf_findFun(Rf_install("getPass"), pass), */
/*                         Rf_lcons(Rf_mkString("Enter passphrase: "), */
/*                                  R_NilValue))); */
/*             nprotect++; */

/*             PROTECT(askpass = Rf_eval(call, pass)); */
/*             nprotect++; */
/*             if (git2r_arg_check_string(askpass) == 0) */
/*                 passphrase = CHAR(STRING_ELT(askpass, 0)); */
/*         } */

/*         error = git_cred_ssh_key_new( */
/*             cred, */
/*             username_from_url, */
/*             public_key, */
/*             private_key, */
/*             passphrase); */

/*         /\* Cleanup. *\/ */
/*         free(private_key); */
/*         free(public_key); */
/*         if (nprotect) */
/*             UNPROTECT(nprotect); */

/*         break; */
/*     } */

/*     if (error) */
/*         return -1; */
/*     return 0; */
/* } */

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
int attribute_hidden
git2r_cred_acquire_cb(
    GIT2R_CREDENTIAL **cred,
    const char *url,
    const char *username_from_url,
    unsigned int allowed_types,
    void *payload)
{
    git2r_transfer_data *td;
    SEXP credentials;

    GIT2R_UNUSED(url);

    if (!payload)
        return -1;

    td = (git2r_transfer_data*)payload;
    credentials = td->credentials;
    if (Rf_isNull(credentials)) {
        if (GIT2R_CREDENTIAL_SSH_KEY & allowed_types) {
	    if (td->use_ssh_agent) {
                /* Try to get credentials from the ssh-agent. */
                td->use_ssh_agent = 0;
                if (GIT2R_CREDENTIAL_SSH_KEY_FROM_AGENT(cred, username_from_url) == 0)
                    return 0;
            }

	    /* if (td->use_ssh_key) { */
            /*     /\* Try to get credentials from default ssh key. *\/ */
            /*     td->use_ssh_key = 0; */
            /*     if (git2r_cred_default_ssh_key(cred, username_from_url) == 0) */
            /*         return 0; */
            /* } */
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
