/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2015 The git2r contributors
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

#include "common.h"

#include "git2r_cred.h"

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
    int err = -1;
    SEXP credentials = R_NilValue;

    GIT_UNUSED(url);

    if (payload)
        credentials = (SEXP)payload;

    if (R_NilValue != credentials) {
        SEXP class_name;

        class_name = getAttrib(credentials, R_ClassSymbol);
        if (0 == strcmp(CHAR(STRING_ELT(class_name, 0)), "cred_ssh_key")) {
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
                if (length(slot)) {
                    if (NA_STRING != STRING_ELT(slot, 0))
                        passphrase = CHAR(STRING_ELT(slot, 0));
                }

                err = git_cred_ssh_key_new(
                    cred,
                    username_from_url,
                    publickey,
                    privatekey,
                    passphrase);
            }
        } else if (0 == strcmp(CHAR(STRING_ELT(class_name, 0)), "cred_user_pass")) {
            if (GIT_CREDTYPE_USERPASS_PLAINTEXT & allowed_types) {
                const char *username;
                const char *password;

                username = CHAR(STRING_ELT(
                                    GET_SLOT(credentials,
                                             Rf_install("username")), 0));
                password = CHAR(STRING_ELT(
                                    GET_SLOT(credentials,
                                             Rf_install("password")), 0));

                err = git_cred_userpass_plaintext_new(cred, username, password);
            }
        }
    } else if (GIT_CREDTYPE_SSH_KEY & allowed_types) {
        err = git_cred_ssh_key_from_agent(cred, username_from_url);
    }

    return err;
}
