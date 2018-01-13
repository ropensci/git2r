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

#include "git2.h"
#include "git2r_arg.h"
#include "git2r_error.h"
#include "git2r_libgit2.h"

/**
 * Return compile time options for libgit2.
 *
 * @return A VECSXP with threads, https and ssh set to TRUE/FALSE
 */
SEXP git2r_libgit2_features(void)
{
    SEXP features, names;
    int value;

    value = git_libgit2_features();
    PROTECT(features = Rf_allocVector(VECSXP, 3));
    setAttrib(features, R_NamesSymbol, names = Rf_allocVector(STRSXP, 3));

    SET_STRING_ELT(names, 0, mkChar("threads"));
    if (value & GIT_FEATURE_THREADS)
        SET_VECTOR_ELT(features, 0, ScalarLogical(1));
    else
        SET_VECTOR_ELT(features, 0, ScalarLogical(0));

    SET_STRING_ELT(names, 1, mkChar("https"));
    if (value & GIT_FEATURE_HTTPS)
        SET_VECTOR_ELT(features, 1, ScalarLogical(1));
    else
        SET_VECTOR_ELT(features, 1, ScalarLogical(0));

    SET_STRING_ELT(names, 2, mkChar("ssh"));
    if (value & GIT_FEATURE_SSH)
        SET_VECTOR_ELT(features, 2, ScalarLogical(1));
    else
        SET_VECTOR_ELT(features, 2, ScalarLogical(0));

    UNPROTECT(1);

    return features;
}

/**
 * Return the version of the libgit2 library being currently used.
 *
 * @return A VECSXP with major, minor and rev.
 */
SEXP git2r_libgit2_version(void)
{
    SEXP version, names;
    int major, minor, rev;

    git_libgit2_version(&major, &minor, &rev);
    PROTECT(version = Rf_allocVector(VECSXP, 3));
    setAttrib(version, R_NamesSymbol, names = Rf_allocVector(STRSXP, 3));
    SET_VECTOR_ELT(version, 0, ScalarInteger(major));
    SET_VECTOR_ELT(version, 1, ScalarInteger(minor));
    SET_VECTOR_ELT(version, 2, ScalarInteger(rev));
    SET_STRING_ELT(names, 0, mkChar("major"));
    SET_STRING_ELT(names, 1, mkChar("minor"));
    SET_STRING_ELT(names, 2, mkChar("rev"));
    UNPROTECT(1);

    return version;
}

/**
 * Set the SSL certificate-authority locations
 *
 * Either parameter may be 'NULL', but not both.
 * @param filename Location of a file containing several certificates
 * concatenated together. Default NULL.
 * @param path Location of a directory holding several certificates,
 * one per file. Default NULL.
 * @return NULL
 */
SEXP git2r_ssl_cert_locations(SEXP filename, SEXP path)
{
    const char *f = NULL;
    const char *p = NULL;

    if (!Rf_isNull(filename)) {
        if (git2r_arg_check_string(filename))
            git2r_error(__func__, NULL, "'filename'", git2r_err_string_arg);
        f = CHAR(STRING_ELT(filename, 0));
    }

    if (!Rf_isNull(path)) {
        if (git2r_arg_check_string(path))
            git2r_error(__func__, NULL, "'path'", git2r_err_string_arg);
        p = CHAR(STRING_ELT(path, 0));
    }

    if (f == NULL && p == NULL)
        git2r_error(__func__, NULL, git2r_err_ssl_cert_locations, NULL);

    if (git_libgit2_opts(GIT_OPT_SET_SSL_CERT_LOCATIONS, f, p))
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return R_NilValue;
}
