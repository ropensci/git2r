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
#include "git2r_blob.h"
#include "git2r_commit.h"
#include "git2r_deprecated.h"
#include "git2r_error.h"
#include "git2r_repository.h"
#include "git2r_S3.h"
#include "git2r_signature.h"
#include "git2r_tag.h"
#include "git2r_tree.h"

/**
 * Init slots in S3 class git_tag
 *
 * @param source a tag
 * @param repo S3 class git_repository that contains the tag
 * @param dest S3 class git_tag to initialize
 * @return void
 */
void attribute_hidden
git2r_tag_init(
    git_tag *source,
    SEXP repo,
    SEXP dest)
{
    const git_signature *tagger;
    const git_oid *oid;
    char sha[GIT_OID_HEXSZ + 1];
    char target[GIT_OID_HEXSZ + 1];

    oid = git_tag_id(source);
    git_oid_tostr(sha, sizeof(sha), oid);
    SET_VECTOR_ELT(dest, git2r_S3_item__git_tag__sha, Rf_mkString(sha));

    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_tag__message,
        Rf_mkString(git_tag_message(source)));

    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_tag__name,
        Rf_mkString(git_tag_name(source)));

    tagger = git_tag_tagger(source);
    if (tagger) {
        if (Rf_isNull(VECTOR_ELT(dest, git2r_S3_item__git_tag__tagger))) {
            SEXP item;

            SET_VECTOR_ELT(
                dest,
                git2r_S3_item__git_tag__tagger,
                item = Rf_mkNamed(VECSXP, git2r_S3_items__git_signature));
            Rf_setAttrib(item, R_ClassSymbol,
                         Rf_mkString(git2r_S3_class__git_signature));
        }
        git2r_signature_init(
            tagger,
            VECTOR_ELT(dest, git2r_S3_item__git_tag__tagger));
    }

    oid = git_tag_target_id(source);
    git_oid_tostr(target, sizeof(target), oid);;
    SET_VECTOR_ELT(dest, git2r_S3_item__git_tag__target, Rf_mkString(target));

    SET_VECTOR_ELT(dest, git2r_S3_item__git_tag__repo, Rf_duplicate(repo));
}

/**
 * Create tag targeting HEAD commit in repository.
 *
 * @param repo S3 class git_repository
 * @param name Name for the tag.
 * @param message The tag message.
 * @param tagger The tagger (author) of the tag
 * @param force Overwrite existing tag.
 * @return S3 object of class git_tag or git_commit
 */
SEXP attribute_hidden
git2r_tag_create(
    SEXP repo,
    SEXP name,
    SEXP message,
    SEXP tagger,
    SEXP force)
{
    SEXP result = R_NilValue;
    int error, nprotect = 0, overwrite = 0;
    git_oid oid;
    git_repository *repository = NULL;
    git_signature *sig_tagger = NULL;
    git_tag *tag = NULL;
    git_object *target = NULL;
    git_commit *commit = NULL;

    if (git2r_arg_check_string(name))
        git2r_error(__func__, NULL, "'name'", git2r_err_string_arg);
    if (!Rf_isNull(message)) {
        if (git2r_arg_check_string(message))
            git2r_error(__func__, NULL, "'message'", git2r_err_string_arg);
        if (git2r_arg_check_signature(tagger))
            git2r_error(__func__, NULL, "'tagger'", git2r_err_signature_arg);
    }
    if (git2r_arg_check_logical(force))
        git2r_error(__func__, NULL, "'force'", git2r_err_logical_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git_revparse_single(&target, repository, "HEAD^{commit}");
    if (error)
        goto cleanup;

    if (LOGICAL(force)[0])
        overwrite = 1;

    if (Rf_isNull(message)) {
        error = git_tag_create_lightweight(
            &oid,
            repository,
            CHAR(STRING_ELT(name, 0)),
            target,
            overwrite);
        if (error)
            goto cleanup;

        error = git_commit_lookup(&commit, repository, &oid);
        if (error)
            goto cleanup;

        PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_commit));
        nprotect++;
        Rf_setAttrib(result, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_commit));
        git2r_commit_init(commit, repo, result);
    } else {
        error = git2r_signature_from_arg(&sig_tagger, tagger);
        if (error)
            goto cleanup;

        error = git_tag_create(
            &oid,
            repository,
            CHAR(STRING_ELT(name, 0)),
            target,
            sig_tagger,
            CHAR(STRING_ELT(message, 0)),
            overwrite);
        if (error)
            goto cleanup;

        error = git_tag_lookup(&tag, repository, &oid);
        if (error)
            goto cleanup;

        PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_tag));
        nprotect++;
        Rf_setAttrib(result, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_tag));
        git2r_tag_init(tag, repo, result);
    }

cleanup:
    git_commit_free(commit);
    git_tag_free(tag);
    git_signature_free(sig_tagger);
    git_object_free(target);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Delete an existing tag reference.
 *
 * @param repo S3 class git_repository
 * @param name Name of the tag to be deleted
 * @return R_NilValue
 */
SEXP attribute_hidden
git2r_tag_delete(
    SEXP repo,
    SEXP name)
{
    int error;
    git_repository *repository = NULL;

    if (git2r_arg_check_string(name))
        git2r_error(__func__, NULL, "'name'", git2r_err_string_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git_tag_delete(repository, CHAR(STRING_ELT(name, 0)));

    git_repository_free(repository);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return R_NilValue;
}

/**
 * Data structure to hold information when iterating over tags.
 */
typedef struct {
    size_t n;
    git_repository *repository;
    SEXP repo;
    SEXP tags;
} git2r_tag_foreach_cb_data;

/**
 * Invoked 'callback' for each tag
 *
 * @param name The name of the tag
 * @param oid The id of the tag
 * @param payload Payload data passed to 'git_tag_foreach'
 * @return 0 on success, else error code
 */
static int
git2r_tag_foreach_cb(
    const char *name,
    git_oid *oid,
    void *payload)
{
    int error = 0;
    git_object *object = NULL;
    git2r_tag_foreach_cb_data *cb_data = (git2r_tag_foreach_cb_data*)payload;

    /* Check if we have a list to populate */
    if (!Rf_isNull(cb_data->tags)) {
        int skip = 0;
        SEXP item = R_NilValue, tag;

        error = git_object_lookup(&object, cb_data->repository, oid, GIT2R_OBJECT_ANY);
        if (error)
            goto cleanup;

        switch (git_object_type(object)) {
        case GIT2R_OBJECT_COMMIT:
            PROTECT(item = Rf_mkNamed(VECSXP, git2r_S3_items__git_commit));
            Rf_setAttrib(
                item,
                R_ClassSymbol,
                Rf_mkString(git2r_S3_class__git_commit));
            git2r_commit_init((git_commit*)object, cb_data->repo, item);
            break;
        case GIT2R_OBJECT_TREE:
            PROTECT(item = Rf_mkNamed(VECSXP, git2r_S3_items__git_tree));
            Rf_setAttrib(
                item,
                R_ClassSymbol,
                Rf_mkString(git2r_S3_class__git_tree));
            git2r_tree_init((git_tree*)object, cb_data->repo, item);
            break;
        case GIT2R_OBJECT_BLOB:
            PROTECT(item = Rf_mkNamed(VECSXP, git2r_S3_items__git_blob));
            Rf_setAttrib(
                item,
                R_ClassSymbol,
                Rf_mkString(git2r_S3_class__git_blob));
            git2r_blob_init((git_blob*)object, cb_data->repo, item);
            break;
        case GIT2R_OBJECT_TAG:
            PROTECT(item = Rf_mkNamed(VECSXP, git2r_S3_items__git_tag));
            Rf_setAttrib(
                item,
                R_ClassSymbol,
                Rf_mkString(git2r_S3_class__git_tag));
            git2r_tag_init((git_tag*)object, cb_data->repo, item);
            break;
        default:
            git2r_error(__func__, NULL, git2r_err_object_type, NULL);
        }

        SET_VECTOR_ELT(cb_data->tags, cb_data->n, item);

        if (strncmp(name, "refs/tags/", sizeof("refs/tags/") - 1) == 0)
            skip = strlen("refs/tags/");
        PROTECT(tag = Rf_mkChar(name + skip));
        SET_STRING_ELT(
            Rf_getAttrib(cb_data->tags, R_NamesSymbol),
            cb_data->n,
            tag);

        UNPROTECT(2);

        if (object)
            git_object_free(object);
        object = NULL;
    }

    cb_data->n += 1;

cleanup:
    git_object_free(object);
    return error;
}

/**
 * Get all tags that can be found in a repository.
 *
 * @param repo S3 class git_repository
 * @return VECXSP with S3 objects of class git_tag
 */
SEXP attribute_hidden
git2r_tag_list(
    SEXP repo)
{
    int error, nprotect = 0;
    SEXP result = R_NilValue;
    git2r_tag_foreach_cb_data cb_data = {0, NULL, R_NilValue, R_NilValue};
    git_repository *repository;

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    /* Count number of tags before creating the list */
    error = git_tag_foreach(repository, &git2r_tag_foreach_cb, &cb_data);
    if (error) {
        if (GIT_ENOTFOUND == error) {
            error = 0;
            PROTECT(result = Rf_allocVector(VECSXP, 0));
            nprotect++;
            Rf_setAttrib(result, R_NamesSymbol, Rf_allocVector(STRSXP, 0));
        }

        goto cleanup;
    }

    PROTECT(result = Rf_allocVector(VECSXP, cb_data.n));
    nprotect++;
    Rf_setAttrib(result, R_NamesSymbol, Rf_allocVector(STRSXP, cb_data.n));

    cb_data.n = 0;
    cb_data.tags = result;
    cb_data.repo = repo;
    cb_data.repository = repository;

    error = git_tag_foreach(repository, &git2r_tag_foreach_cb, &cb_data);

cleanup:
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}
