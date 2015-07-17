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

#include "git2r_arg.h"
#include "git2r_error.h"
#include "git2r_repository.h"
#include "git2r_signature.h"
#include "git2r_tag.h"

/**
 * Init slots in S4 class git_tag
 *
 * @param source a tag
 * @param repo S4 class git_repository that contains the tag
 * @param dest S4 class git_tag to initialize
 * @return void
 */
void git2r_tag_init(git_tag *source, SEXP repo, SEXP dest)
{
    const git_signature *tagger;
    const git_oid *oid;
    char sha[GIT_OID_HEXSZ + 1];
    char target[GIT_OID_HEXSZ + 1];

    oid = git_tag_id(source);
    git_oid_tostr(sha, sizeof(sha), oid);
    SET_SLOT(dest, Rf_install("sha"), mkString(sha));

    SET_SLOT(dest, Rf_install("message"), mkString(git_tag_message(source)));
    SET_SLOT(dest, Rf_install("name"), mkString(git_tag_name(source)));

    tagger = git_tag_tagger(source);
    if (tagger)
        git2r_signature_init(tagger, GET_SLOT(dest, Rf_install("tagger")));

    oid = git_tag_target_id(source);
    git_oid_tostr(target, sizeof(target), oid);;
    SET_SLOT(dest, Rf_install("target"), mkString(target));

    SET_SLOT(dest, Rf_install("repo"), repo);
}

/**
 * Create tag targeting HEAD commit in repository.
 *
 * @param repo S4 class git_repository
 * @param name Name for the tag.
 * @param message The tag message.
 * @param tagger The tagger (author) of the tag
 * @return S4 object of class git_tag
 */
SEXP git2r_tag_create(SEXP repo, SEXP name, SEXP message, SEXP tagger)
{
    SEXP result = R_NilValue;
    int err;
    git_oid oid;
    git_repository *repository = NULL;
    git_signature *sig_tagger = NULL;
    git_tag *tag = NULL;
    git_object *target = NULL;

    if (git2r_arg_check_string(name))
        git2r_error(__func__, NULL, "'name'", git2r_err_string_arg);
    if (git2r_arg_check_string(message))
        git2r_error(__func__, NULL, "'message'", git2r_err_string_arg);
    if (git2r_arg_check_signature(tagger))
        git2r_error(__func__, NULL, "'tagger'", git2r_err_signature_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git2r_signature_from_arg(&sig_tagger, tagger);
    if (err)
        goto cleanup;

    err = git_revparse_single(&target, repository, "HEAD^{commit}");
    if (err)
        goto cleanup;

    err = git_tag_create(
        &oid,
        repository,
        CHAR(STRING_ELT(name, 0)),
        target,
        sig_tagger,
        CHAR(STRING_ELT(message, 0)),
        0);
    if (err)
        goto cleanup;

    err = git_tag_lookup(&tag, repository, &oid);
    if (err)
        goto cleanup;

    PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_tag")));
    git2r_tag_init(tag, repo, result);

cleanup:
    if (tag)
        git_tag_free(tag);

    if (sig_tagger)
        git_signature_free(sig_tagger);

    if (target)
        git_object_free(target);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return result;
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
 * @param name The name of the tag, git_oid *oid, void *payload
 * @param oid The id of the tag
 * @param payload Payload data passed to 'git_tag_foreach'
 * @return 0 on success, else error code
 */
static int git2r_tag_foreach_cb(const char *name, git_oid *oid, void *payload)
{
    int err = 0;
    git_tag *tag = NULL;
    git2r_tag_foreach_cb_data *cb_data = (git2r_tag_foreach_cb_data*)payload;

    /* Check if we have a list to populate */
    if (R_NilValue != cb_data->tags) {
        SEXP tag_item;

        err = git_tag_lookup(&tag, cb_data->repository, oid);
        if (err)
            goto cleanup;

        SET_VECTOR_ELT(
            cb_data->tags,
            cb_data->n,
            tag_item = NEW_OBJECT(MAKE_CLASS("git_tag")));
        git2r_tag_init(tag, cb_data->repo, tag_item);
        SET_STRING_ELT(
            getAttrib(cb_data->tags, R_NamesSymbol),
            cb_data->n,
            STRING_ELT(GET_SLOT(tag_item, Rf_install("name")), 0));

        if (tag)
            git_tag_free(tag);
        tag = NULL;
    }

    cb_data->n += 1;

cleanup:
    if (tag)
        git_tag_free(tag);

    return err;
}

/**
 * Get all tags that can be found in a repository.
 *
 * @param repo S4 class git_repository
 * @return VECXSP with S4 objects of class git_tag
 */
SEXP git2r_tag_list(SEXP repo)
{
    int err;
    SEXP result = R_NilValue;
    git2r_tag_foreach_cb_data cb_data = {0, NULL, R_NilValue, R_NilValue};
    git_repository *repository;

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    /* Count number of tags before creating the list */
    err = git_tag_foreach(repository, &git2r_tag_foreach_cb, &cb_data);
    if (err) {
        if (GIT_ENOTFOUND == err) {
            err = 0;
            PROTECT(result = allocVector(VECSXP, 0));
            setAttrib(result, R_NamesSymbol, allocVector(STRSXP, 0));
        }

        goto cleanup;
    }

    PROTECT(result = allocVector(VECSXP, cb_data.n));
    setAttrib(result, R_NamesSymbol, allocVector(STRSXP, cb_data.n));

    cb_data.n = 0;
    cb_data.tags = result;
    cb_data.repo = repo;
    cb_data.repository = repository;

    err = git_tag_foreach(repository, &git2r_tag_foreach_cb, &cb_data);

cleanup:
    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return result;
}
