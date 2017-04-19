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
#include "buffer.h"

#include "git2r_arg.h"
#include "git2r_error.h"
#include "git2r_note.h"
#include "git2r_repository.h"
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
 * Init slots in S4 class git_note
 *
 * @param blob_id Oid of the blob containing the message
 * @param annotated_object_id Oid of the git object being annotated
 * @param repository
 * @param repo S4 class git_repository that contains the stash
 * @param dest S4 class git_note to initialize
 * @return int 0 on success, or an error code.
 */
static int git2r_note_init(
    const git_oid *blob_id,
    const git_oid *annotated_object_id,
    git_repository *repository,
    const char *notes_ref,
    SEXP repo,
    SEXP dest)
{
    int err;
    git_note *note = NULL;
    char sha[GIT_OID_HEXSZ + 1];
    SEXP s_sha = Rf_install("sha");
    SEXP s_annotated = Rf_install("annotated");
    SEXP s_message = Rf_install("message");
    SEXP s_refname = Rf_install("refname");
    SEXP s_repo = Rf_install("repo");

    err = git_note_read(&note, repository, notes_ref, annotated_object_id);
    if (err)
        return err;

    git_oid_fmt(sha, blob_id);
    sha[GIT_OID_HEXSZ] = '\0';
    SET_SLOT(dest, s_sha, mkString(sha));
    git_oid_fmt(sha, annotated_object_id);
    sha[GIT_OID_HEXSZ] = '\0';
    SET_SLOT(dest, s_annotated, mkString(sha));
    SET_SLOT(dest, s_message, mkString(git_note_message(note)));
    SET_SLOT(dest, s_refname, mkString(notes_ref));
    SET_SLOT(dest, s_repo, repo);

    if (note)
        git_note_free(note);

    return 0;
}

/**
 * Add a note for an object
 *
 * @param repo S4 class git_repository
 * @param sha The sha string of object
 * @param commit S4 class git_commit
 * @param message Content of the note to add
 * @param ref Canonical name of the reference to use
 * @param author Signature of the notes note author
 * @param committer Signature of the notes note committer
 * @param force Overwrite existing note
 * @return S4 class git_note
 */
SEXP git2r_note_create(
    SEXP repo,
    SEXP sha,
    SEXP message,
    SEXP ref,
    SEXP author,
    SEXP committer,
    SEXP force)
{
    int err;
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

    err = git2r_signature_from_arg(&sig_author, author);
    if (err)
        goto cleanup;

    err = git2r_signature_from_arg(&sig_committer, committer);
    if (err)
        goto cleanup;

    err = git_oid_fromstr(&object_oid, CHAR(STRING_ELT(sha, 0)));
    if (err)
        goto cleanup;

    if (LOGICAL(force)[0])
        overwrite = 1;

    err = git_note_create(
        &note_oid,
        repository,
        CHAR(STRING_ELT(ref, 0)),
        sig_author,
        sig_committer,
        &object_oid,
        CHAR(STRING_ELT(message, 0)),
        overwrite);
    if (err)
        goto cleanup;

    PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_note")));
    err = git2r_note_init(&note_oid,
                          &object_oid,
                          repository,
                          CHAR(STRING_ELT(ref, 0)),
                          repo,
                          result);

cleanup:
    if (sig_author)
        git_signature_free(sig_author);

    if (sig_committer)
        git_signature_free(sig_committer);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return result;
}

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
    git_buf buf = GIT_BUF_INIT;
    git_repository *repository = NULL;

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    err = git_note_default_ref(&buf, repository);
    if (err)
        goto cleanup;

    PROTECT(result = allocVector(STRSXP, 1));
    SET_STRING_ELT(result, 0, mkChar(buf.ptr));

cleanup:
    git_buf_free(&buf);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

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
static int git2r_note_foreach_cb(
    const git_oid *blob_id,
    const git_oid *annotated_object_id,
    void *payload)
{
    git2r_note_foreach_cb_data *cb_data = (git2r_note_foreach_cb_data*)payload;

    /* Check if we have a list to populate */
    if (R_NilValue != cb_data->list) {
        int err;
        SEXP note;

        SET_VECTOR_ELT(
            cb_data->list,
            cb_data->n,
            note = NEW_OBJECT(MAKE_CLASS("git_note")));

        err = git2r_note_init(
            blob_id,
            annotated_object_id,
            cb_data->repository,
            cb_data->notes_ref,
            cb_data->repo,
            note);
        if (err)
            return err;
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
SEXP git2r_notes(SEXP repo, SEXP ref)
{
    int err;
    SEXP result = R_NilValue;
    git_buf buf = GIT_BUF_INIT;
    git2r_note_foreach_cb_data cb_data = {0, R_NilValue, R_NilValue, NULL, NULL};
    git_repository *repository = NULL;

    if (R_NilValue != ref) {
        if (git2r_arg_check_string(ref))
            git2r_error(__func__, NULL, "'ref'", git2r_err_string_arg);
    }

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    if (R_NilValue != ref) {
        git_buf_sets(&buf, CHAR(STRING_ELT(ref, 0)));
    } else {
        err = git_note_default_ref(&buf, repository);
        if (err)
            goto cleanup;
    }

    /* Count number of notes before creating the list */
    err = git_note_foreach(
        repository,
        git_buf_cstr(&buf),
        &git2r_note_foreach_cb,
        &cb_data);
    if (err) {
        if (GIT_ENOTFOUND == err) {
            err = GIT_OK;
            PROTECT(result = allocVector(VECSXP, 0));
        }

        goto cleanup;
    }

    PROTECT(result = allocVector(VECSXP, cb_data.n));
    cb_data.n = 0;
    cb_data.list = result;
    cb_data.repo = repo;
    cb_data.repository = repository;
    cb_data.notes_ref = git_buf_cstr(&buf);
    err = git_note_foreach(repository, git_buf_cstr(&buf),
                           &git2r_note_foreach_cb, &cb_data);

cleanup:
    git_buf_free(&buf);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return result;
}

/**
 * Remove the note for an object
 *
 * @param note S4 class git_note
 * @param author Signature of the notes commit author
 * @param committer Signature of the notes commit committer
 * @return R_NilValue
 */
SEXP git2r_note_remove(SEXP note, SEXP author, SEXP committer)
{
    int err;
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

    repo = GET_SLOT(note, Rf_install("repo"));
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    err = git2r_signature_from_arg(&sig_author, author);
    if (err)
        goto cleanup;

    err = git2r_signature_from_arg(&sig_committer, committer);
    if (err)
        goto cleanup;

    annotated = GET_SLOT(note, Rf_install("annotated"));
    err = git_oid_fromstr(&note_oid, CHAR(STRING_ELT(annotated, 0)));
    if (err)
        goto cleanup;

    err = git_note_remove(
        repository,
        CHAR(STRING_ELT(GET_SLOT(note, Rf_install("refname")), 0)),
        sig_author,
        sig_committer,
        &note_oid);

cleanup:
    if (sig_author)
        git_signature_free(sig_author);

    if (sig_committer)
        git_signature_free(sig_committer);

    if (repository)
        git_repository_free(repository);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return R_NilValue;
}
