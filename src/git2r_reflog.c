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
#include "git2r_reflog.h"
#include "git2r_repository.h"
#include "git2r_S3.h"
#include "git2r_signature.h"

/**
 * Init slots in S3 class git_reflog_entry
 *
 * @param source The source reflog entry
 * @param index The entry index
 * @param repo S3 class git_repository
 * @param ref Reference to read from
 * @param dest S3 class git_reflog_entry to initialize
 * @return void
 */
static void
git2r_reflog_entry_init(
    const git_reflog_entry *source,
    size_t index,
    SEXP repo,
    SEXP ref,
    SEXP dest)
{
    SEXP i;
    const char *message;
    const git_signature *committer;
    char sha[GIT_OID_HEXSZ + 1];

    git_oid_fmt(sha, git_reflog_entry_id_new(source));
    sha[GIT_OID_HEXSZ] = '\0';
    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_reflog_entry__sha,
        Rf_mkString(sha));

    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_reflog_entry__index,
        i = Rf_allocVector(INTSXP, 1));
    INTEGER(i)[0] = index;

    committer = git_reflog_entry_committer(source);
    if (committer) {
        if (Rf_isNull(VECTOR_ELT(dest, git2r_S3_item__git_reflog_entry__committer))) {
            SEXP item;

            SET_VECTOR_ELT(
                dest,
                git2r_S3_item__git_reflog_entry__committer,
                item = Rf_mkNamed(VECSXP, git2r_S3_items__git_signature));
            Rf_setAttrib(item, R_ClassSymbol,
                         Rf_mkString(git2r_S3_class__git_signature));
        }
        git2r_signature_init(
            committer,
            VECTOR_ELT(dest, git2r_S3_item__git_reflog_entry__committer));
    }

    message = git_reflog_entry_message(source);
    if (message) {
        SET_VECTOR_ELT(
            dest,
            git2r_S3_item__git_reflog_entry__message,
            Rf_mkString(message));
    } else {
        SET_VECTOR_ELT(
            dest,
            git2r_S3_item__git_reflog_entry__message,
            Rf_ScalarString(NA_STRING));
    }

    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_reflog_entry__refname,
        ref);

    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_reflog_entry__repo,
        Rf_duplicate(repo));
}

/**
 * List the reflog within a specified reference.
 *
 * @param repo S3 class git_repository
 * @param ref Reference to read from.
 * @return VECXSP with S3 objects of class git_reflog
 */
SEXP attribute_hidden
git2r_reflog_list(
    SEXP repo,
    SEXP ref)
{
    int error, nprotect = 0;
    size_t i, n;
    SEXP result = R_NilValue;
    git_reflog *reflog = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_string(ref))
        git2r_error(__func__, NULL, "'ref'", git2r_err_string_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git_reflog_read(&reflog, repository, CHAR(STRING_ELT(ref, 0)));
    if (error)
        goto cleanup;

    n = git_reflog_entrycount(reflog);
    PROTECT(result = Rf_allocVector(VECSXP, n));
    nprotect++;

    for (i = 0; i < n; i++) {
        const git_reflog_entry *entry = git_reflog_entry_byindex(reflog, i);

        if (entry) {
            SEXP item;

            SET_VECTOR_ELT(
                result,
                i,
                item = Rf_mkNamed(VECSXP, git2r_S3_items__git_reflog_entry));
            Rf_setAttrib(item, R_ClassSymbol,
                         Rf_mkString(git2r_S3_class__git_reflog_entry));
            git2r_reflog_entry_init(entry, i, repo, ref, item);
        }
    }

cleanup:
    git_reflog_free(reflog);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}
