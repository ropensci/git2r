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
#include "git2r_stash.h"
#include "git2.h"

typedef struct {
    size_t n;
    SEXP list;
    SEXP repo;
    git_repository *repository;
} stashes_cb_data;

/**
 * Callback when iterating over stashes
 *
 * @param index
 * @param message
 * @param stash_id
 * @param payload
 * @return int
 */
static int stashes_cb(
    size_t index,
    const char* message,
    const git_oid *stash_id,
    void *payload)
{
    int err = 0;
    char hex[GIT_OID_HEXSZ + 1];
    SEXP stash;
    SEXP stasher;
    git_commit *commit = NULL;
    stashes_cb_data *cb_data = (stashes_cb_data*)payload;

    /* Check if we have a list to populate */
    if (R_NilValue != cb_data->list) {
        err = git_commit_lookup(&commit, cb_data->repository, stash_id);
        if (err < 0)
            return err;

        PROTECT(stash = NEW_OBJECT(MAKE_CLASS("git_stash")));

        git_oid_fmt(hex, stash_id);
        hex[GIT_OID_HEXSZ] = '\0';
        SET_SLOT(stash,
                 Rf_install("hex"),
                 ScalarString(mkChar(hex)));

        SET_SLOT(stash,
                 Rf_install("message"),
                 ScalarString(mkChar(message)));
        
        PROTECT(stasher = NEW_OBJECT(MAKE_CLASS("git_signature")));
        init_signature(git_commit_committer(commit), stasher);
        SET_SLOT(stash, Rf_install("stasher"), stasher);
        UNPROTECT(1);
        git_commit_free(commit);

        SET_SLOT(stash, Rf_install("repo"), duplicate(cb_data->repo));

        SET_VECTOR_ELT(cb_data->list, cb_data->n, stash);
        UNPROTECT(1);
    }

    cb_data->n += 1;

    return 0;
}

/**
 * List stashes in a repository
 *
 * @param repo S4 class git_repository
 * @return VECXSP with S4 objects of class git_stash
 */
SEXP stashes(SEXP repo)
{
    SEXP list = R_NilValue;
    int err = 0;
    const char* err_msg = NULL;
    stashes_cb_data cb_data = {0, R_NilValue, R_NilValue, NULL};
    git_repository *repository = NULL;

    repository = get_repository(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    /* Count number of stashes before creating the list */
    git_stash_foreach(repository, &stashes_cb, &cb_data);
    if (err < 0)
        goto cleanup;

    PROTECT(list = allocVector(VECSXP, cb_data.n));
    cb_data.n = 0;
    cb_data.list = list;
    cb_data.repo = repo;
    cb_data.repository = repository;
    err = git_stash_foreach(repository, &stashes_cb, &cb_data);

cleanup:
    if (repository)
        git_repository_free(repository);

    if (R_NilValue != list)
        UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return list;
}
