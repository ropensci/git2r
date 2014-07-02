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
#include "git2r_reflog.h"
#include "git2r_repository.h"
#include "git2r_signature.h"

/**
 * Init slots in S4 class git_reflog_entry
 *
 * @param source The source reflog entry
 * @param index The entry index
 * @param repo S4 class git_repository
 * @param ref Reference to read from
 * @param dest S4 class git_reflog_entry to initialize
 * @return void
 */
void git2r_reflog_entry_init(
    git_reflog_entry *source,
    size_t index,
    SEXP repo,
    SEXP ref,
    SEXP dest)
{
    SEXP sexp_index;
    const char *message;
    const git_signature *committer;
    char hex[GIT_OID_HEXSZ + 1];

    git_oid_fmt(hex, git_reflog_entry_id_new(source));
    hex[GIT_OID_HEXSZ] = '\0';
    SET_SLOT(dest,
             Rf_install("hex"),
             ScalarString(mkChar(hex)));

    PROTECT(sexp_index = allocVector(INTSXP, 1));
    INTEGER(sexp_index)[0] = index;
    SET_SLOT(dest, Rf_install("index"), sexp_index);
    UNPROTECT(1);

    committer = git_reflog_entry_committer(source);
    if (committer) {
        SEXP sexp_committer;

        PROTECT(sexp_committer = NEW_OBJECT(MAKE_CLASS("git_signature")));
        git2r_signature_init(committer, sexp_committer);
        SET_SLOT(dest, Rf_install("committer"), sexp_committer);
        UNPROTECT(1);
    }

    message = git_reflog_entry_message(source);
    if (message) {
        SET_SLOT(dest,
                 Rf_install("message"),
                 ScalarString(mkChar(message)));
    } else {
        SET_SLOT(dest,
                 Rf_install("message"),
                 ScalarString(NA_STRING));
    }


    SET_SLOT(dest, Rf_install("refname"), duplicate(ref));
    SET_SLOT(dest, Rf_install("repo"), duplicate(repo));
}

/**
 * List the reflog within a specified reference.
 *
 * @param repo S4 class git_repository
 * @param ref Reference to read from.
 * @return VECXSP with S4 objects of class git_reflog
 */
SEXP git2r_reflog_list(SEXP repo, SEXP ref)
{
    int err;
    size_t i, n;
    SEXP result = R_NilValue;
    git_reflog *reflog = NULL;
    git_repository *repository = NULL;

    if (git2r_error_check_string_arg(ref))
        error("Invalid arguments to git2r_reflog_list");

    repository = git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    err = git_reflog_read(&reflog, repository, CHAR(STRING_ELT(ref, 0)));
    if (err < 0)
        goto cleanup;

    n = git_reflog_entrycount(reflog);
    PROTECT(result = allocVector(VECSXP, n));
    for (i = 0; i < n; i++) {
        git_reflog_entry *reflog_entry = git_reflog_entry_byindex(reflog, i);

        if (reflog_entry) {
            SEXP entry;

            PROTECT(entry = NEW_OBJECT(MAKE_CLASS("git_reflog_entry")));
            git2r_reflog_entry_init(reflog_entry, i, repo, ref, entry);
            SET_VECTOR_ELT(result, i, entry);
            UNPROTECT(1);
        }
    }

cleanup:
    if (reflog)
        git_reflog_free(reflog);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return result;
}
