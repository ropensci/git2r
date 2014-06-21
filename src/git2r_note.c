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

#include "git2r_error.h"
#include "git2r_note.h"
#include "git2r_repository.h"
#include "git2.h"

typedef struct {
    size_t n;
    SEXP list;
    SEXP repo;
    git_repository *repository;
    const char *notes_ref;
} git2r_note_list_cb_data;

/**
 * Default notes reference
 *
 * Get the default notes reference for a repository
 * @param repo S4 class git_repository
 * @return Character vector of length one with name of default
 * reference
*/
SEXP git2r_note_default_ref(SEXP repo)
{
    int err;
    SEXP result = R_NilValue;
    const char *ref;
    git_repository *repository = NULL;

    repository = git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    err = git_note_default_ref(&ref, repository);
    if (err < 0)
        goto cleanup;

    PROTECT(result = allocVector(STRSXP, 1));
    SET_STRING_ELT(result, 0, mkChar(ref));

cleanup:
    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return result;
}

/**
 * Init slots in S4 class git_note
 *
 * @param blob_id Oid of the blob containing the message
 * @param annotated_object_id Oid of the git object being annotated
 * @param repository
 * @param repo S4 class git_repository that contains the stash
 * @param dest S4 class git_note to initialize
 * @return int 0 on success, or an error code.
 */
int git2r_note_init(
    const git_oid *blob_id,
    const git_oid *annotated_object_id,
    git_repository *repository,
    const char *notes_ref,
    SEXP repo,
    SEXP dest)
{
    int err;
    git_note *note = NULL;
    char hex[GIT_OID_HEXSZ + 1];

    err = git_note_read(&note, repository, notes_ref, annotated_object_id);
    if (err < 0)
        return err;

    git_oid_fmt(hex, blob_id);
    hex[GIT_OID_HEXSZ] = '\0';
    SET_SLOT(dest,
             Rf_install("hex"),
             ScalarString(mkChar(hex)));

    git_oid_fmt(hex, annotated_object_id);
    hex[GIT_OID_HEXSZ] = '\0';
    SET_SLOT(dest,
             Rf_install("annotated"),
             ScalarString(mkChar(hex)));

    SET_SLOT(dest,
             Rf_install("message"),
             ScalarString(mkChar(git_note_message(note))));

    SET_SLOT(dest,
             Rf_install("refname"),
             ScalarString(mkChar(notes_ref)));

    SET_SLOT(dest, Rf_install("repo"), duplicate(repo));

    if (note)
        git_note_free(note);

    return 0;
}

/**
 * Callback when iterating over notes
 *
 * @param blob_id Oid of the blob containing the message
 * @param annotated_object_id Oid of the git object being annotated
 * @param payload Payload data passed to `git_note_foreach`
 * @return int 0 or error code
 */
static int git2r_note_list_cb(
    const git_oid *blob_id,
    const git_oid *annotated_object_id,
    void *payload)
{
    int err;
    SEXP note;
    git2r_note_list_cb_data *cb_data = (git2r_note_list_cb_data*)payload;

    /* Check if we have a list to populate */
    if (R_NilValue != cb_data->list) {
        PROTECT(note = NEW_OBJECT(MAKE_CLASS("git_note")));
        err = git2r_note_init(blob_id,
                              annotated_object_id,
                              cb_data->repository,
                              cb_data->notes_ref,
                              cb_data->repo,
                              note);
        if (err < 0) {
            UNPROTECT(1);
            return err;
        }
        SET_VECTOR_ELT(cb_data->list, cb_data->n, note);
        UNPROTECT(1);
    }

    cb_data->n += 1;

    return 0;
}

/**
 * List all the notes within a specified namespace.
 *
 * @param repo S4 class git_repository
 * @param ref Optional reference to read from.
 * @return VECXSP with S4 objects of class git_note
 */
SEXP git2r_note_list(SEXP repo, SEXP ref)
{
    int err;
    SEXP result = R_NilValue;
    const char *notes_ref = NULL;
    git2r_note_list_cb_data cb_data = {0, R_NilValue, R_NilValue, NULL, NULL};
    git_repository *repository = NULL;

    if (R_NilValue != ref) {
        if (git2r_error_check_string_arg(ref))
            error("Invalid arguments to git2r_note_list");
        notes_ref = CHAR(STRING_ELT(ref, 0));
    }

    repository = git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    if (NULL == notes_ref) {
        err = git_note_default_ref(&notes_ref, repository);
        if (err < 0)
            goto cleanup;
    }

    /* Count number of notes before creating the list */
    err = git_note_foreach(repository, notes_ref, &git2r_note_list_cb, &cb_data);
    if (err < 0) {
        if (GIT_ENOTFOUND == err) {
            err = 0;
            PROTECT(result = allocVector(VECSXP, 0));
        }

        goto cleanup;
    }

    PROTECT(result = allocVector(VECSXP, cb_data.n));
    cb_data.n = 0;
    cb_data.list = result;
    cb_data.repo = repo;
    cb_data.repository = repository;
    cb_data.notes_ref = notes_ref;
    err = git_note_foreach(repository, notes_ref, &git2r_note_list_cb, &cb_data);

cleanup:
    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return result;
}
