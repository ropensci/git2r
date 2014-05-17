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

#include "git2r_commit.h"
#include "git2r_error.h"
#include "git2r_repository.h"
#include "git2r_signature.h"
#include "git2r_stash.h"
#include "git2.h"

typedef struct {
    size_t n;
    SEXP list;
    SEXP repo;
    git_repository *repository;
} stashes_cb_data;

/**
 * Remove a stash from the stash list
 *
 * @param repo S4 class git_repository that contains the stash
 * @param index The index to the stash. 0 is the most recent stash.
 * @return R_NilValue
 */
SEXP drop_stash(SEXP repo, SEXP index)
{
    int err;
    git_repository *repository = NULL;

    if (git2r_check_integer_arg(index))
        error("Invalid arguments to drop_stash");
    if (0 > INTEGER(index)[0])
        error("'index' out of range");
    repository = get_repository(repo);
    if (!repository)
        error(git2r_err_invalid_repository);
    err = git_stash_drop(repository, INTEGER(index)[0]);
    git_repository_free(repository);
    if (err < 0)
        error("Error: %s\n", giterr_last()->message);
    return R_NilValue;
}

/**
 * Init slots in S4 class git_stash
 *
 * @param source
 * @param repository
 * @param repo S4 class git_repository that contains the stash
 * @param dest S4 class git_stash to initialize
 * @return int 0 on success, or an error code.
 */
int init_stash(
    const git_oid *source,
    git_repository *repository,
    SEXP repo,
    SEXP dest)
{
    int err;
    git_commit *commit = NULL;

    err = git_commit_lookup(&commit, repository, source);
    if (err < 0)
        return err;
    git2r_init_commit(commit, repo, dest);
    git_commit_free(commit);

    return 0;
}

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
    stashes_cb_data *cb_data = (stashes_cb_data*)payload;

    /* Check if we have a list to populate */
    if (R_NilValue != cb_data->list) {
        int err;
        SEXP stash;

        PROTECT(stash = NEW_OBJECT(MAKE_CLASS("git_stash")));
        err = init_stash(stash_id,
                         cb_data->repository,
                         cb_data->repo,
                         stash);
        if (err < 0) {
            UNPROTECT(1);
            return err;
        }
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

/**
 * Stash
 *
 * @param repo The repository
 * @param message Optional description
 * @param index All changes already added to the index are left
 *        intact in the working directory. Default is FALSE
 * @param untracked All untracked files are also stashed and then
 *        cleaned up from the working directory. Default is FALSE
 * @param ignored All ignored files are also stashed and then cleaned
 *        up from the working directory. Default is FALSE
 * @param stasher Signature with stasher and time of stash
 * @return S4 class git_stash
 */
SEXP stash(
    SEXP repo,
    SEXP message,
    SEXP index,
    SEXP untracked,
    SEXP ignored,
    SEXP stasher)
{
    int err;
    SEXP when;
    SEXP sexp_stash = R_NilValue;
    git_oid oid;
    git_stash_flags flags = GIT_STASH_DEFAULT;
    git_commit *commit = NULL;
    git_repository *repository = NULL;
    git_signature *sig_stasher = NULL;

    if (git2r_check_logical_arg(index)
        || git2r_check_logical_arg(untracked)
        || git2r_check_logical_arg(ignored)
        || git2r_check_signature_arg(stasher))
        error("Invalid arguments to stash");

    repository = get_repository(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    if (LOGICAL(index)[0])
        flags |= GIT_STASH_KEEP_INDEX;
    if (LOGICAL(untracked)[0])
        flags |= GIT_STASH_INCLUDE_UNTRACKED;
    if (LOGICAL(ignored)[0])
        flags |= GIT_STASH_INCLUDE_IGNORED;

    when = GET_SLOT(stasher, Rf_install("when"));
    err = git_signature_new(&sig_stasher,
                            CHAR(STRING_ELT(GET_SLOT(stasher, Rf_install("name")), 0)),
                            CHAR(STRING_ELT(GET_SLOT(stasher, Rf_install("email")), 0)),
                            REAL(GET_SLOT(when, Rf_install("time")))[0],
                            REAL(GET_SLOT(when, Rf_install("offset")))[0]);
    if (err < 0)
        goto cleanup;

    err = git_stash_save(&oid,
                         repository,
                         sig_stasher,
                         CHAR(STRING_ELT(message, 0)),
                         flags);
    if (GIT_ENOTFOUND == err) {
        err = 0;
        goto cleanup;
    } else if (err < 0) {
        goto cleanup;
    }

    PROTECT(sexp_stash = NEW_OBJECT(MAKE_CLASS("git_stash")));
    err = init_stash(&oid, repository, repo, sexp_stash);

cleanup:
    if (commit)
        git_commit_free(commit);

    if (sig_stasher)
        git_signature_free(sig_stasher);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != sexp_stash)
        UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return sexp_stash;
}
