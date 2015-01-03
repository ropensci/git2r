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
    SEXP sexp_index;
    const char *message;
    const git_signature *committer;
    char sha[GIT_OID_HEXSZ + 1];

    git_oid_fmt(sha, git_reflog_entry_id_new(source));
    sha[GIT_OID_HEXSZ] = '\0';
    SET_SLOT(dest,
             Rf_install("sha"),
             ScalarString(mkChar(sha)));

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
    if (message)
        SET_SLOT(dest, Rf_install("message"), ScalarString(mkChar(message)));
    else
        SET_SLOT(dest, Rf_install("message"), ScalarString(NA_STRING));

    SET_SLOT(dest, Rf_install("refname"), ref);
    SET_SLOT(dest, Rf_install("repo"), repo);
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

    if (GIT_OK != git2r_arg_check_string(ref))
        git2r_error(git2r_err_string_arg, __func__, "ref");

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git_reflog_read(&reflog, repository, CHAR(STRING_ELT(ref, 0)));
    if (GIT_OK != err)
        goto cleanup;

    n = git_reflog_entrycount(reflog);
    PROTECT(result = allocVector(VECSXP, n));
    for (i = 0; i < n; i++) {
        const git_reflog_entry *reflog_entry = git_reflog_entry_byindex(reflog, i);

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

    if (GIT_OK != err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return result;
}
