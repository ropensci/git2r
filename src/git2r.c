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

/** @file git2r.c
 *  @brief R bindings to the libgit2 library
 *
 *  These functions are called from R with .Call to interface the
 *  libgit2 library from R.
 *
 */

#include <Rdefines.h>
#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>

#include "git2.h"

#include "git2r_blob.h"
#include "git2r_checkout.h"
#include "git2r_clone.h"
#include "git2r_config.h"
#include "git2r_commit.h"
#include "git2r_error.h"
#include "git2r_repository.h"
#include "git2r_push.h"
#include "git2r_signature.h"
#include "git2r_stash.h"
#include "git2r_status.h"
#include "git2r_tag.h"

static void init_reference(git_reference *ref, SEXP reference);
static int number_of_branches(git_repository *repo, int flags, size_t *n);

/**
 * Add files to a repository
 *
 * @param repo S4 class git_repository
 * @param path
 * @return R_NilValue
 */
SEXP add(SEXP repo, SEXP path)
{
    int err;
    git_index *index = NULL;
    git_repository *repository = NULL;

    if (check_string_arg(path))
        error("Invalid arguments to add");

    repository= get_repository(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    err = git_repository_index(&index, repository);
    if (err < 0)
        goto cleanup;

    err = git_index_add_bypath(index, CHAR(STRING_ELT(path, 0)));
    if (err < 0)
        goto cleanup;

    err = git_index_write(index);
    if (err < 0)
        goto cleanup;

cleanup:
    if (index)
        git_index_free(index);

    if (repository)
        git_repository_free(repository);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return R_NilValue;
}

/**
 * List branches in a repository
 *
 * @param repo S4 class git_repository
 * @return VECXSP with S4 objects of class git_branch
 */
SEXP branches(SEXP repo, SEXP flags)
{
    SEXP list;
    int err = 0;
    const char* err_msg = NULL;
    git_branch_iterator *iter = NULL;
    size_t i = 0, n = 0;
    size_t protected = 0;
    git_repository *repository = NULL;

    repository = get_repository(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    /* Count number of branches before creating the list */
    err = number_of_branches(repository, INTEGER(flags)[0], &n);
    if (err < 0)
        goto cleanup;

    PROTECT(list = allocVector(VECSXP, n));
    protected++;

    err = git_branch_iterator_new(&iter, repository,  INTEGER(flags)[0]);
    if (err < 0)
        goto cleanup;

    for (;;) {
        SEXP branch;
        git_branch_t type;
        git_reference *ref;
        const char *refname;

        err = git_branch_next(&ref, &type, iter);
        if (err < 0) {
            if (GIT_ITEROVER == err) {
                err = 0;
                break;
            }
            goto cleanup;
        }

        PROTECT(branch = NEW_OBJECT(MAKE_CLASS("git_branch")));
        protected++;

        refname = git_reference_name(ref);
        init_reference(ref, branch);

        switch (type) {
        case GIT_BRANCH_LOCAL:
            break;
        case GIT_BRANCH_REMOTE: {
            git_buf buf = {0};
            git_remote *remote = NULL;

            err = git_branch_remote_name(&buf, repository, refname);
            if (err < 0)
                goto cleanup;
            SET_SLOT(branch, Rf_install("remote"), ScalarString(mkChar(buf.ptr)));

            err = git_remote_load(&remote, repository, buf.ptr);
            if (err < 0) {
                err = git_remote_create_anonymous(&remote, repository, buf.ptr, NULL);
                if (err < 0) {
                    git_buf_free(&buf);
                    goto cleanup;
                }
            }

            SET_SLOT(branch,
                     Rf_install("url"),
                     ScalarString(mkChar(git_remote_url(remote))));

            git_buf_free(&buf);
            git_remote_free(remote);
            break;
        }
        default:
            err = -1;
            err_msg = git2r_err_unexpected_type_of_branch;
            goto cleanup;
        }

        switch (git_branch_is_head(ref)) {
        case 0:
            SET_SLOT(branch, Rf_install("head"), ScalarLogical(0));
            break;
        case 1:
            SET_SLOT(branch, Rf_install("head"), ScalarLogical(1));
            break;
        default:
            err = -1;
            err_msg = git2r_err_unexpected_head_of_branch;
            goto cleanup;
        }

        git_reference_free(ref);
        SET_VECTOR_ELT(list, i, branch);
        UNPROTECT(1);
        protected--;
        i++;
    }

cleanup:
    if (iter)
        git_branch_iterator_free(iter);

    if (repository)
        git_repository_free(repository);

    if (protected)
        UNPROTECT(protected);

    if (err < 0) {
        if (err_msg)
            error(err_msg);
        else
            error("Error: %s\n", giterr_last()->message);
    }

    return list;
}

/**
 * Fetch
 *
 * @param repo
 * @param name
 * @return R_NilValue
 */
SEXP fetch(SEXP repo, SEXP name)
{
    int err;
    git_remote *remote = NULL;
    git_repository *repository = NULL;

    if (check_string_arg(name))
        error("Invalid arguments to fetch");

    repository = get_repository(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    err = git_remote_load(&remote, repository, CHAR(STRING_ELT(name, 0)));
    if (err < 0)
        goto cleanup;

    err = git_remote_fetch(remote, NULL, NULL);
    if (err < 0)
        goto cleanup;

cleanup:
    if (remote)
        git_remote_disconnect(remote);

    if (remote)
        git_remote_free(remote);

    if (repository)
        git_repository_free(repository);

    return R_NilValue;
}

/**
 * Init slots in S4 class git_reference.
 *
 * @param ref
 * @param reference
 * @return void
 */
static void init_reference(git_reference *ref, SEXP reference)
{
    char out[41];
    out[40] = '\0';

    SET_SLOT(reference,
             Rf_install("name"),
             ScalarString(mkChar(git_reference_name(ref))));

    SET_SLOT(reference,
             Rf_install("shorthand"),
             ScalarString(mkChar(git_reference_shorthand(ref))));

    switch (git_reference_type(ref)) {
    case GIT_REF_OID:
        SET_SLOT(reference, Rf_install("type"), ScalarInteger(GIT_REF_OID));
        git_oid_fmt(out, git_reference_target(ref));
        SET_SLOT(reference, Rf_install("hex"), ScalarString(mkChar(out)));
        break;
    case GIT_REF_SYMBOLIC:
        SET_SLOT(reference, Rf_install("type"), ScalarInteger(GIT_REF_SYMBOLIC));
        SET_SLOT(reference,
                 Rf_install("target"),
                 ScalarString(mkChar(git_reference_symbolic_target(ref))));
        break;
    default:
        error("Unexpected reference type");
    }
}

/**
 * Count number of branches.
 *
 * @param repo S4 class git_repository
 * @param flags
 * @return
 */
static int number_of_branches(git_repository *repo, int flags, size_t *n)
{
    int err;
    git_branch_iterator *iter;
    git_branch_t type;
    git_reference *ref;

    *n = 0;

    err = git_branch_iterator_new(&iter, repo, flags);
    if (err < 0)
        return err;

    for (;;) {
        err = git_branch_next(&ref, &type, iter);
        if (err < 0)
            break;
        git_reference_free(ref);
        (*n)++;
    }

    git_branch_iterator_free(iter);

    if (GIT_ITEROVER != err)
        return err;
    return 0;
}

/**
 * Count number of revisions.
 *
 * @param walker
 * @return
 */
static size_t number_of_revisions(git_revwalk *walker)
{
    size_t n = 0;
    git_oid oid;

    while (!git_revwalk_next(&oid, walker))
        n++;
    return n;
}

/**
 * Get all references that can be found in a repository.
 *
 * @param repo S4 class git_repository
 * @return VECXSP with S4 objects of class git_reference
 */
SEXP references(SEXP repo)
{
    int i, err;
    git_strarray ref_list;
    SEXP list = R_NilValue;
    SEXP names = R_NilValue;
    git_reference *ref;
    git_repository *repository;

    repository = get_repository(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    err = git_reference_list(&ref_list, repository);
    if (err < 0)
        goto cleanup;

    PROTECT(list = allocVector(VECSXP, ref_list.count));
    PROTECT(names = allocVector(STRSXP, ref_list.count));

    for (i = 0; i < ref_list.count; i++) {
        SEXP reference;

        err = git_reference_lookup(&ref, repository, ref_list.strings[i]);
        if (err < 0)
            goto cleanup;

        PROTECT(reference = NEW_OBJECT(MAKE_CLASS("git_reference")));
        init_reference(ref, reference);
        SET_STRING_ELT(names, i, mkChar(ref_list.strings[i]));
        SET_VECTOR_ELT(list, i, reference);
        UNPROTECT(1);
    }

cleanup:
    git_strarray_free(&ref_list);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != list && R_NilValue != names) {
        setAttrib(list, R_NamesSymbol, names);
        UNPROTECT(2);
    }

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return list;
}

/**
 * Get the configured remotes for a repo
 *
 * @param repo S4 class git_repository
 * @return
 */
SEXP remotes(SEXP repo)
{
    int i, err;
    git_strarray rem_list;
    SEXP list;
    git_repository *repository;

    repository = get_repository(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    err = git_remote_list(&rem_list, repository);
    if (err < 0)
        goto cleanup;

    PROTECT(list = allocVector(STRSXP, rem_list.count));
    for (i = 0; i < rem_list.count; i++)
        SET_STRING_ELT(list, i, mkChar(rem_list.strings[i]));
    UNPROTECT(1);

cleanup:
    git_strarray_free(&rem_list);

    if (repository)
        git_repository_free(repository);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return list;
}

/**
 * Get the remote's url
 *
 * @param repo S4 class git_repository
 * @return
 */
SEXP remote_url(SEXP repo, SEXP remote)
{
    int err;
    SEXP url;
    size_t len = LENGTH(remote);
    size_t i = 0;
    git_repository *repository;

    repository = get_repository(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    PROTECT(url = allocVector(STRSXP, len));

    for (; i < len; i++) {
        git_remote *r;

        err = git_remote_load(&r, repository, CHAR(STRING_ELT(remote, i)));
        if (err < 0)
            goto cleanup;

        SET_STRING_ELT(url, i, mkChar(git_remote_url(r)));
        git_remote_free(r);
    }

cleanup:
    git_repository_free(repository);

    UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return url;
}

/**
 * List revisions
 *
 * @param repo S4 class git_repository
 * @return
 */
SEXP revisions(SEXP repo)
{
    int i=0;
    int err = 0;
    SEXP list;
    size_t n = 0;
    git_revwalk *walker = NULL;
    git_repository *repository = NULL;

    repository = get_repository(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    if (git_repository_is_empty(repository)) {
        /* No commits, create empty list */
        PROTECT(list = allocVector(VECSXP, 0));
        goto cleanup;
    }

    err = git_revwalk_new(&walker, repository);
    if (err < 0)
        goto cleanup;

    err = git_revwalk_push_head(walker);
    if (err < 0)
        goto cleanup;

    /* Count number of revisions before creating the list */
    n = number_of_revisions(walker);

    /* Create list to store result */
    PROTECT(list = allocVector(VECSXP, n));

    git_revwalk_reset(walker);
    err = git_revwalk_push_head(walker);
    if (err < 0)
        goto cleanup;

    for (;;) {
        git_commit *commit;
        SEXP sexp_commit;
        git_oid oid;

        err = git_revwalk_next(&oid, walker);
        if (err < 0) {
            if (GIT_ITEROVER == err)
                err = 0;
            goto cleanup;
        }

        err = git_commit_lookup(&commit, repository, &oid);
        if (err < 0)
            goto cleanup;

        PROTECT(sexp_commit = NEW_OBJECT(MAKE_CLASS("git_commit")));
        init_commit(commit, repo, sexp_commit);
        SET_VECTOR_ELT(list, i, sexp_commit);
        UNPROTECT(1);
        i++;

        git_commit_free(commit);
    }

cleanup:
    if (walker)
        git_revwalk_free(walker);

    if (repository)
        git_repository_free(repository);

    UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return list;
}

static const R_CallMethodDef callMethods[] =
{
    {"add", (DL_FUNC)&add, 2},
    {"branches", (DL_FUNC)&branches, 2},
    {"checkout_commit", (DL_FUNC)&checkout_commit, 1},
    {"checkout_tag", (DL_FUNC)&checkout_tag, 1},
    {"checkout_tree", (DL_FUNC)&checkout_tree, 1},
    {"clone", (DL_FUNC)&clone, 2},
    {"commit", (DL_FUNC)&commit, 5},
    {"descendant_of", (DL_FUNC)&descendant_of, 2},
    {"default_signature", (DL_FUNC)&default_signature, 1},
    {"drop_stash", (DL_FUNC)&drop_stash, 2},
    {"fetch", (DL_FUNC)&fetch, 2},
    {"get_config", (DL_FUNC)&get_config, 1},
    {"init", (DL_FUNC)&init, 2},
    {"is_bare", (DL_FUNC)&is_bare, 1},
    {"git2r_is_binary", (DL_FUNC)&git2r_is_binary, 1},
    {"is_detached", (DL_FUNC)&is_detached, 1},
    {"is_empty", (DL_FUNC)&is_empty, 1},
    {"is_repository", (DL_FUNC)&is_repository, 1},
    {"lookup", (DL_FUNC)&lookup, 2},
    {"parents", (DL_FUNC)&parents, 1},
    {"push", (DL_FUNC)&push, 3},
    {"git2r_rawsize", (DL_FUNC)&git2r_rawsize, 1},
    {"references", (DL_FUNC)&references, 1},
    {"remotes", (DL_FUNC)&remotes, 1},
    {"remote_url", (DL_FUNC)&remote_url, 2},
    {"revisions", (DL_FUNC)&revisions, 1},
    {"set_config", (DL_FUNC)&set_config, 2},
    {"stash", (DL_FUNC)&stash, 6},
    {"stashes", (DL_FUNC)&stashes, 1},
    {"status", (DL_FUNC)&status, 5},
    {"tag", (DL_FUNC)&tag, 4},
    {"tags", (DL_FUNC)&tags, 1},
    {"workdir", (DL_FUNC)&workdir, 1},
    {NULL, NULL, 0}
};

/**
 * Register routines to R.
 *
 * @param info Information about the DLL being loaded
 */
void
R_init_gitr(DllInfo *info)
{
    R_registerRoutines(info, NULL, callMethods, NULL, NULL);
}
