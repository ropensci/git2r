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
#include "git2r_note.h"
#include "git2r_repository.h"
#include "git2r_S3.h"
#include "git2r_signature.h"

/**
 * Data structure to hold information when iterating over note
 * objects.
 */
typedef struct {
    size_t n;
    SEXP list;
    SEXP repo;
    git_repository *repository;
    const char *notes_ref;
} git2r_note_foreach_cb_data;

/**
 * Init slots in S3 class git_note
 *
 * @param blob_id Oid of the blob containing the message
 * @param annotated_object_id Oid of the git object being annotated
 * @param repository
 * @param repo S3 class git_repository that contains the stash
 * @param dest S3 class git_note to initialize
 * @return int 0 on success, or an error code.
 */
static int
git2r_note_init(
    const git_oid *blob_id,
    const git_oid *annotated_object_id,
    git_repository *repository,
    const char *notes_ref,
    SEXP repo,
    SEXP dest)
{
    int error;
    git_note *note = NULL;
    char sha[GIT_OID_HEXSZ + 1];

    error = git_note_read(&note, repository, notes_ref, annotated_object_id);
    if (error)
        return error;

    git_oid_fmt(sha, blob_id);
    sha[GIT_OID_HEXSZ] = '\0';
    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_note__sha,
        Rf_mkString(sha));

    git_oid_fmt(sha, annotated_object_id);
    sha[GIT_OID_HEXSZ] = '\0';
    SET_VECTOR_ELT(dest, git2r_S3_item__git_note__annotated, Rf_mkString(sha));

    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_note__message,
        Rf_mkString(git_note_message(note)));

    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_note__refname,
        Rf_mkString(notes_ref));

    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_note__repo,
        Rf_duplicate(repo));

    git_note_free(note);

    return 0;
}

/**
 * Add a note for an object
 *
 * @param repo S3 class git_repository
 * @param sha The sha string of object
 * @param commit S3 class git_commit
 * @param message Content of the note to add
 * @param ref Canonical name of the reference to use
 * @param author Signature of the notes note author
 * @param committer Signature of the notes note committer
 * @param force Overwrite existing note
 * @return S3 class git_note
 */
SEXP attribute_hidden
git2r_note_create(
    SEXP repo,
    SEXP sha,
    SEXP message,
    SEXP ref,
    SEXP author,
    SEXP committer,
    SEXP force)
{
    int error, nprotect = 0;
    SEXP result = R_NilValue;
    int overwrite = 0;
    git_oid note_oid;
    git_oid object_oid;
    git_signature *sig_author = NULL;
    git_signature *sig_committer = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_sha(sha))
        git2r_error(__func__, NULL, "'sha'", git2r_err_sha_arg);
    if (git2r_arg_check_string(message))
        git2r_error(__func__, NULL, "'message'", git2r_err_string_arg);
    if (git2r_arg_check_string(ref))
        git2r_error(__func__, NULL, "'ref'", git2r_err_string_arg);
    if (git2r_arg_check_signature(author))
        git2r_error(__func__, NULL, "'author'", git2r_err_signature_arg);
    if (git2r_arg_check_signature(committer))
        git2r_error(__func__, NULL, "'committer'", git2r_err_signature_arg);
    if (git2r_arg_check_logical(force))
        git2r_error(__func__, NULL, "'force'", git2r_err_logical_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git2r_signature_from_arg(&sig_author, author);
    if (error)
        goto cleanup;

    error = git2r_signature_from_arg(&sig_committer, committer);
    if (error)
        goto cleanup;

    error = git_oid_fromstr(&object_oid, CHAR(STRING_ELT(sha, 0)));
    if (error)
        goto cleanup;

    if (LOGICAL(force)[0])
        overwrite = 1;

    error = git_note_create(
        &note_oid,
        repository,
        CHAR(STRING_ELT(ref, 0)),
        sig_author,
        sig_committer,
        &object_oid,
        CHAR(STRING_ELT(message, 0)),
        overwrite);
    if (error)
        goto cleanup;

    PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_note));
    nprotect++;
    Rf_setAttrib(result, R_ClassSymbol,
                 Rf_mkString(git2r_S3_class__git_note));
    error = git2r_note_init(&note_oid,
                          &object_oid,
                          repository,
                          CHAR(STRING_ELT(ref, 0)),
                          repo,
                          result);

cleanup:
    git_signature_free(sig_author);
    git_signature_free(sig_committer);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Default notes reference
 *
 * Get the default notes reference for a repository
 * @param repo S3 class git_repository
 * @return Character vector of length one with name of default
 * reference
 */
SEXP attribute_hidden
git2r_note_default_ref(
    SEXP repo)
{
    int error, nprotect = 0;
    SEXP result = R_NilValue;
    git_buf buf = GIT_BUF_INIT_CONST(NULL, 0);
    git_repository *repository = NULL;

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git_note_default_ref(&buf, repository);
    if (error)
        goto cleanup;

    PROTECT(result = Rf_allocVector(STRSXP, 1));
    nprotect++;
    SET_STRING_ELT(result, 0, Rf_mkChar(buf.ptr));

cleanup:
    GIT2R_BUF_DISPOSE(&buf);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Callback when iterating over notes
 *
 * @param blob_id Oid of the blob containing the message
 * @param annotated_object_id Oid of the git object being annotated
 * @param payload Payload data passed to `git_note_foreach`
 * @return int 0 or error code
 */
static int
git2r_note_foreach_cb(
    const git_oid *blob_id,
    const git_oid *annotated_object_id,
    void *payload)
{
    int error = 0, nprotect = 0;
    SEXP note;
    git2r_note_foreach_cb_data *cb_data = (git2r_note_foreach_cb_data*)payload;

    /* Check if we have a list to populate */
    if (!Rf_isNull(cb_data->list)) {
        PROTECT(note = Rf_mkNamed(VECSXP, git2r_S3_items__git_note));
        nprotect++;
        Rf_setAttrib(
            note,
            R_ClassSymbol,
            Rf_mkString(git2r_S3_class__git_note));

        error = git2r_note_init(
            blob_id,
            annotated_object_id,
            cb_data->repository,
            cb_data->notes_ref,
            cb_data->repo,
            note);
        if (error)
            goto cleanup;

        SET_VECTOR_ELT(cb_data->list, cb_data->n, note);
    }

    cb_data->n += 1;

cleanup:
    if (nprotect)
        UNPROTECT(nprotect);

    return error;
}

/**
 * List all the notes within a specified namespace.
 *
 * @param repo S3 class git_repository
 * @param ref Optional reference to read from.
 * @return VECXSP with S3 objects of class git_note
 */
SEXP attribute_hidden
git2r_notes(
    SEXP repo,
    SEXP ref)
{
    int error, nprotect = 0;
    SEXP result = R_NilValue;
    git_buf buf = GIT_BUF_INIT_CONST(NULL, 0);
    const char *notes_ref;
    git2r_note_foreach_cb_data cb_data = {0, R_NilValue, R_NilValue, NULL, NULL};
    git_repository *repository = NULL;

    if (!Rf_isNull(ref)) {
        if (git2r_arg_check_string(ref))
            git2r_error(__func__, NULL, "'ref'", git2r_err_string_arg);
    }

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    if (!Rf_isNull(ref)) {
        notes_ref = CHAR(STRING_ELT(ref, 0));
    } else {
        error = git_note_default_ref(&buf, repository);
        if (error)
            goto cleanup;
        notes_ref = buf.ptr;
    }

    /* Count number of notes before creating the list */
    error = git_note_foreach(
        repository,
        notes_ref,
        &git2r_note_foreach_cb,
        &cb_data);
    if (error) {
        if (GIT_ENOTFOUND == error) {
            error = GIT_OK;
            PROTECT(result = Rf_allocVector(VECSXP, 0));
            nprotect++;
        }

        goto cleanup;
    }

    PROTECT(result = Rf_allocVector(VECSXP, cb_data.n));
    nprotect++;
    cb_data.n = 0;
    cb_data.list = result;
    cb_data.repo = repo;
    cb_data.repository = repository;
    cb_data.notes_ref = notes_ref;
    error = git_note_foreach(repository, notes_ref,
                           &git2r_note_foreach_cb, &cb_data);

cleanup:
    GIT2R_BUF_DISPOSE(&buf);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Remove the note for an object
 *
 * @param note S3 class git_note
 * @param author Signature of the notes commit author
 * @param committer Signature of the notes commit committer
 * @return R_NilValue
 */
SEXP attribute_hidden
git2r_note_remove(
    SEXP note,
    SEXP author,
    SEXP committer)
{
    int error;
    SEXP repo;
    SEXP annotated;
    git_oid note_oid;
    git_signature *sig_author = NULL;
    git_signature *sig_committer = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_note(note))
        git2r_error(__func__, NULL, "'note'", git2r_err_note_arg);
    if (git2r_arg_check_signature(author))
        git2r_error(__func__, NULL, "'author'", git2r_err_signature_arg);
    if (git2r_arg_check_signature(committer))
        git2r_error(__func__, NULL, "'committer'", git2r_err_signature_arg);

    repo = git2r_get_list_element(note, "repo");
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git2r_signature_from_arg(&sig_author, author);
    if (error)
        goto cleanup;

    error = git2r_signature_from_arg(&sig_committer, committer);
    if (error)
        goto cleanup;

    annotated = git2r_get_list_element(note, "annotated");
    error = git_oid_fromstr(&note_oid, CHAR(STRING_ELT(annotated, 0)));
    if (error)
        goto cleanup;

    error = git_note_remove(
        repository,
        CHAR(STRING_ELT(git2r_get_list_element(note, "refname"), 0)),
        sig_author,
        sig_committer,
        &note_oid);

cleanup:
    git_signature_free(sig_author);
    git_signature_free(sig_committer);
    git_repository_free(repository);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return R_NilValue;
}
