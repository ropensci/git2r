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
#include "git2r_repository.h"
#include "git2r_signature.h"
#include "git2r_tag.h"

/**
 * Init slots in S4 class git_tag
 *
 * @param source a tag
 * @param dest S4 class git_tag to initialize
 * @return void
 */
void init_tag(git_tag *source, SEXP dest)
{
    int err;
    const git_signature *tagger;
    const git_oid *oid;
    char hex[GIT_OID_HEXSZ + 1];
    char target[GIT_OID_HEXSZ + 1];

    oid = git_tag_id(source);
    git_oid_tostr(hex, sizeof(hex), oid);
    SET_SLOT(dest,
             Rf_install("hex"),
             ScalarString(mkChar(hex)));

    SET_SLOT(dest,
             Rf_install("message"),
             ScalarString(mkChar(git_tag_message(source))));

    SET_SLOT(dest,
             Rf_install("name"),
             ScalarString(mkChar(git_tag_name(source))));

    tagger = git_tag_tagger(source);
    if (tagger) {
        SEXP sexp_tagger;

        PROTECT(sexp_tagger = NEW_OBJECT(MAKE_CLASS("git_signature")));
        init_signature(tagger, sexp_tagger);
        SET_SLOT(dest, Rf_install("tagger"), sexp_tagger);
        UNPROTECT(1);
    }

    oid = git_tag_target_id(source);
    git_oid_tostr(target, sizeof(target), oid);;
    SET_SLOT(dest,
             Rf_install("target"),
             ScalarString(mkChar(target)));
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
SEXP tag(const SEXP repo, const SEXP name, const SEXP message, const SEXP tagger)
{
    SEXP when;
    SEXP sexp_tag;
    int err;
    size_t protected = 0;
    git_oid oid;
    git_repository *repository = NULL;
    git_signature *sig_tagger = NULL;
    git_tag *new_tag = NULL;
    git_object *target = NULL;

    if (check_string_arg(name)
        || check_string_arg(message)
        || check_signature_arg(tagger))
        error("Invalid arguments to tag");

    when = GET_SLOT(tagger, Rf_install("when"));
    err = git_signature_new(&sig_tagger,
                            CHAR(STRING_ELT(GET_SLOT(tagger, Rf_install("name")), 0)),
                            CHAR(STRING_ELT(GET_SLOT(tagger, Rf_install("email")), 0)),
                            REAL(GET_SLOT(when, Rf_install("time")))[0],
                            REAL(GET_SLOT(when, Rf_install("offset")))[0]);
    if (err < 0)
        goto cleanup;

    repository = get_repository(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    err = git_revparse_single(&target, repository, "HEAD^{commit}");
    if (err < 0)
        goto cleanup;

    err = git_tag_create(&oid,
                         repository,
                         CHAR(STRING_ELT(name, 0)),
                         target,
                         sig_tagger,
                         CHAR(STRING_ELT(message, 0)),
                         0);
    if (err < 0)
        goto cleanup;

    err = git_tag_lookup(&new_tag, repository, &oid);
    if (err < 0)
        goto cleanup;

    PROTECT(sexp_tag = NEW_OBJECT(MAKE_CLASS("git_tag")));
    protected++;
    init_tag(new_tag, sexp_tag);

cleanup:
    if (new_tag)
        git_tag_free(new_tag);

    if (sig_tagger)
        git_signature_free(sig_tagger);

    if (target)
        git_object_free(target);

    if (repository)
        git_repository_free(repository);

    if (protected)
        unprotect(protected);

    if (err < 0) {
        const git_error *e = giterr_last();
        error("Error: %s\n", e->message);
    }

    return sexp_tag;
}

/**
 * Get all tags that can be found in a repository.
 *
 * @param repo S4 class git_repository
 * @return VECXSP with S4 objects of class git_tag
 */
SEXP tags(const SEXP repo)
{
    int err;
    SEXP list;
    size_t protected = 0;
    git_repository *repository;
    git_reference* reference = NULL;
    git_tag *tag = NULL;
    git_strarray tag_names = {0};
    size_t i;

    repository = get_repository(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    err = git_tag_list(&tag_names, repository);
    if (err < 0)
        goto cleanup;

    PROTECT(list = allocVector(VECSXP, tag_names.count));
    protected++;

    for(i = 0; i < tag_names.count; i++) {
        SEXP sexp_tag;
        const git_oid *oid;

        err = git_reference_dwim(&reference, repository, tag_names.strings[i]);
        if (err < 0)
            goto cleanup;

        oid = git_reference_target(reference);
        err = git_tag_lookup(&tag, repository, oid);
        if (err < 0)
            goto cleanup;

        PROTECT(sexp_tag = NEW_OBJECT(MAKE_CLASS("git_tag")));
        protected++;
        init_tag(tag, sexp_tag);

        SET_VECTOR_ELT(list, i, sexp_tag);
        UNPROTECT(1);
        protected--;

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

    if (protected)
        UNPROTECT(protected);

    if (err < 0) {
        const git_error *e = giterr_last();
        error("Error %d: %s\n", e->klass, e->message);
    }

    return list;
}
