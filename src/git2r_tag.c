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

    if (GIT_OK != git2r_arg_check_string(name))
        git2r_error(git2r_err_string_arg, __func__, "name");
    if (GIT_OK != git2r_arg_check_string(message))
        git2r_error(git2r_err_string_arg, __func__, "message");
    if (GIT_OK != git2r_arg_check_signature(tagger))
        git2r_error(git2r_err_signature_arg, __func__, "tagger");

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git2r_signature_from_arg(&sig_tagger, tagger);
    if (GIT_OK != err)
        goto cleanup;

    err = git_revparse_single(&target, repository, "HEAD^{commit}");
    if (GIT_OK != err)
        goto cleanup;

    err = git_tag_create(
        &oid,
        repository,
        CHAR(STRING_ELT(name, 0)),
        target,
        sig_tagger,
        CHAR(STRING_ELT(message, 0)),
        0);
    if (GIT_OK != err)
        goto cleanup;

    err = git_tag_lookup(&tag, repository, &oid);
    if (GIT_OK != err)
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

    if (GIT_OK != err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return result;
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
    SEXP list = R_NilValue;
    git_repository *repository;
    git_reference* reference = NULL;
    git_tag *tag = NULL;
    git_strarray tag_names = {0};
    size_t i;

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git_tag_list(&tag_names, repository);
    if (GIT_OK != err)
        goto cleanup;

    PROTECT(list = allocVector(VECSXP, tag_names.count));

    for(i = 0; i < tag_names.count; i++) {
        SEXP sexp_tag;
        const git_oid *oid;

        err = git_reference_dwim(&reference, repository, tag_names.strings[i]);
        if (GIT_OK != err)
            goto cleanup;

        oid = git_reference_target(reference);
        err = git_tag_lookup(&tag, repository, oid);
        if (GIT_OK != err)
            goto cleanup;

        SET_VECTOR_ELT(list, i, sexp_tag = NEW_OBJECT(MAKE_CLASS("git_tag")));
        git2r_tag_init(tag, repo, sexp_tag);

        git_tag_free(tag);
        tag = NULL;
        git_reference_free(reference);
        reference = NULL;
    }

cleanup:
    git_strarray_free(&tag_names);

    if (tag)
        git_tag_free(tag);

    if (reference)
        git_reference_free(reference);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != list)
        UNPROTECT(1);

    if (GIT_OK != err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return list;
}
