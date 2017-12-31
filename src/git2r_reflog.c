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

#include <Rdefines.h>
#include "git2.h"

#include "git2r_arg.h"
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
    SEXP s_sha = Rf_install("sha");
    SEXP s_index = Rf_install("index");
    SEXP s_committer = Rf_install("committer");
    SEXP s_message = Rf_install("message");
    SEXP s_refname = Rf_install("refname");
    SEXP s_repo = Rf_install("repo");

    git_oid_fmt(sha, git_reflog_entry_id_new(source));
    sha[GIT_OID_HEXSZ] = '\0';
    SET_SLOT(dest, s_sha, mkString(sha));

    SET_SLOT(dest, s_index, i = allocVector(INTSXP, 1));
    INTEGER(i)[0] = index;

    committer = git_reflog_entry_committer(source);
    if (committer)
        git2r_signature_init(committer, GET_SLOT(dest, s_committer));

    message = git_reflog_entry_message(source);
    if (message)
        SET_SLOT(dest, s_message, mkString(message));
    else
        SET_SLOT(dest, s_message, ScalarString(NA_STRING));

    SET_SLOT(dest, s_refname, ref);
    SET_SLOT(dest, s_repo, repo);
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

    if (git2r_arg_check_string(ref))
        git2r_error(__func__, NULL, "'ref'", git2r_err_string_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    err = git_reflog_read(&reflog, repository, CHAR(STRING_ELT(ref, 0)));
    if (err)
        goto cleanup;

    n = git_reflog_entrycount(reflog);
    PROTECT(result = allocVector(VECSXP, n));
    for (i = 0; i < n; i++) {
        const git_reflog_entry *entry = git_reflog_entry_byindex(reflog, i);

        if (entry) {
            SEXP item;

            SET_VECTOR_ELT(result,
                           i,
                           item = NEW_OBJECT(MAKE_CLASS("git_reflog_entry")));
            git2r_reflog_entry_init(entry, i, repo, ref, item);
        }
    }

cleanup:
    if (reflog)
        git_reflog_free(reflog);

    if (repository)
        git_repository_free(repository);

    if (!isNull(result))
        UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return result;
}
