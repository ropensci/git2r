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
#include "git2r_tag.h"

/**
 * Init slots in S4 class git_tag
 *
 * @param source a tag
 * @param dest S4 class git_tag to initialize
 * @return void
 */
static void init_tag(git_tag *source, SEXP dest)
{
    int err;
    const git_signature *tagger;
    const git_oid *oid;
    char target[GIT_OID_HEXSZ + 1];

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
