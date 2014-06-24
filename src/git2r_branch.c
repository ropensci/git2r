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
#include "git2r_commit.h"
#include "git2r_error.h"
#include "git2r_reference.h"
#include "git2r_repository.h"
#include "git2r_signature.h"

/**
 * Count number of branches.
 *
 * @param repo S4 class git_repository
 * @param flags Filtering flags for the branch listing. Valid values
 *        are 1 (LOCAL), 2 (REMOTE) and 3 (ALL)
 * @param n
 * @return
 */
static int git2r_branch_count(git_repository *repo, int flags, size_t *n)
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
 * Init slots in S4 class git_branch
 *
 * @param source a reference
 * @param repository the repository
 * @param type the branch type; local or remote
 * @param repo S4 class git_repository that contains the blob
 * @param dest S4 class git_branch to initialize
 * @return int; < 0 if error, else 0
 */
static int git2r_branch_init(
    const git_reference *source,
    git_branch_t type,
    SEXP repo,
    SEXP dest)
{
    int err = 0;
    const char *name;

    err = git_branch_name(&name, source);
    if (err < 0)
        goto cleanup;
    SET_SLOT(dest,
             Rf_install("name"),
             ScalarString(mkChar(name)));

    SET_SLOT(dest, Rf_install("type"), ScalarInteger(type));
    SET_SLOT(dest, Rf_install("repo"), duplicate(repo));

cleanup:
    return err;
}

/**
 * Create a new branch
 *
 * @param branch_name Name for the branch
 * @param commit Commit to which branch should point.
 * @param force Overwrite existing branch
 * @param signature The identity that will be used to populate the reflog entry
 * @param message The one line long message to the reflog. If NULL, default is
 *        "Branch: created"
 * @return S4 class git_branch
 */
SEXP git2r_branch_create(
    SEXP branch_name,
    SEXP commit,
    SEXP force,
    SEXP signature,
    SEXP message)
{
    SEXP repo;
    SEXP result = R_NilValue;
    int err;
    int overwrite = 0;
    const char *log = NULL;
    git_commit *target = NULL;
    git_signature *who = NULL;
    git_reference *reference = NULL;
    git_repository *repository = NULL;

    if (git2r_error_check_string_arg(branch_name)
        || git2r_error_check_commit_arg(commit)
        || git2r_error_check_logical_arg(force)
        || git2r_error_check_signature_arg(signature))
        error("Invalid arguments to git2r_branch_create");

    if (R_NilValue != message) {
        if (git2r_error_check_string_arg(message))
            error("Invalid arguments to git2r_branch_create");
        log = CHAR(STRING_ELT(message, 0));
    }

    repo = GET_SLOT(commit, Rf_install("repo"));
    repository = git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    err = git2r_commit_lookup(&target, repository, commit);
    if (err < 0)
        goto cleanup;

    if (LOGICAL(force)[0])
        overwrite = 1;

    err = git2r_signature_from_arg(&who, signature);
    if (err < 0)
        goto cleanup;

    err = git_branch_create(
        &reference,
        repository,
        CHAR(STRING_ELT(branch_name, 0)),
        target,
        overwrite,
        who,
        log);
    if (err)
        goto cleanup;

    PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_branch")));
    err = git2r_branch_init(reference, GIT_BRANCH_LOCAL, repo, result);

cleanup:
    if (reference)
        git_reference_free(reference);

    if (target)
        git_commit_free(target);

    if (who)
        git_signature_free(who);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return result;
}

/**
 * Delete branch
 *
 * @param branch S4 class git_branch
 * @return R_NilValue
 */
SEXP git2r_branch_delete(SEXP branch)
{
    int err;
    const char *name;
    git_branch_t type;
    git_reference *reference = NULL;
    git_repository *repository = NULL;

    if (git2r_error_check_branch_arg(branch))
        error("Invalid arguments to git2r_branch_is_head");

    repository = git2r_repository_open(GET_SLOT(branch, Rf_install("repo")));
    if (!repository)
        error(git2r_err_invalid_repository);

    name = CHAR(STRING_ELT(GET_SLOT(branch, Rf_install("name")), 0));
    type = INTEGER(GET_SLOT(branch, Rf_install("type")))[0];
    err = git_branch_lookup(&reference, repository, name, type);
    if (err < 0)
        goto cleanup;

    err = git_branch_delete(reference);

cleanup:
    if (reference)
        git_reference_free(reference);

    if (repository)
        git_repository_free(repository);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return R_NilValue;
}

/**
 * Determine if the current local branch is pointed at by HEAD
 *
 * @param branch S4 class git_branch
 * @return TRUE if head, FALSE if not
 */
SEXP git2r_branch_is_head(SEXP branch)
{
    SEXP result = R_NilValue;
    int err;
    const char *name;
    git_reference *reference = NULL;
    git_repository *repository = NULL;

    if (git2r_error_check_branch_arg(branch))
        error("Invalid arguments to git2r_branch_is_head");

    repository = git2r_repository_open(GET_SLOT(branch, Rf_install("repo")));
    if (!repository)
        error(git2r_err_invalid_repository);

    name = CHAR(STRING_ELT(GET_SLOT(branch, Rf_install("name")), 0));

    err = git_branch_lookup(&reference,
                            repository,
                            name,
                            INTEGER(GET_SLOT(branch, Rf_install("type")))[0]);
    if (err < 0)
        goto cleanup;

    err = git_branch_is_head(reference);
    if (err < 0)
        goto cleanup;

    if (0 == err || 1 == err) {
        PROTECT(result = allocVector(LGLSXP, 1));
        LOGICAL(result)[0] = err;
    }

cleanup:
    if (reference)
        git_reference_free(reference);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return result;
}

/**
 * List branches in a repository
 *
 * @param repo S4 class git_repository
 * @param flags Filtering flags for the branch listing. Valid values
 *        are 1 (LOCAL), 2 (REMOTE) and 3 (ALL)
 * @return VECXSP with S4 objects of class git_branch
 */
SEXP git2r_branch_list(SEXP repo, SEXP flags)
{
    SEXP list = R_NilValue;
    int err = 0;
    git_branch_iterator *iter = NULL;
    size_t i = 0, n = 0;
    git_repository *repository = NULL;
    git_reference *reference = NULL;
    git_branch_t type;

    if (git2r_error_check_integer_arg(flags))
        error("Invalid arguments to git2r_branch_list");

    repository = git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    /* Count number of branches before creating the list */
    err = git2r_branch_count(repository, INTEGER(flags)[0], &n);
    if (err < 0)
        goto cleanup;
    PROTECT(list = allocVector(VECSXP, n));

    err = git_branch_iterator_new(&iter, repository,  INTEGER(flags)[0]);
    if (err < 0)
        goto cleanup;

    for (;;) {
        SEXP branch;

        err = git_branch_next(&reference, &type, iter);
        if (err < 0) {
            if (GIT_ITEROVER == err) {
                err = 0;
                break;
            }
            goto cleanup;
        }

        PROTECT(branch = NEW_OBJECT(MAKE_CLASS("git_branch")));
        err = git2r_branch_init(reference, type, repo, branch);
        if (err < 0) {
            UNPROTECT(1);
            goto cleanup;
        }
        SET_VECTOR_ELT(list, i, branch);
        UNPROTECT(1);
        if (reference)
            git_reference_free(reference);
        reference = NULL;
        i++;
    }

cleanup:
    if (iter)
        git_branch_iterator_free(iter);

    if (reference)
        git_reference_free(reference);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != list)
        UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return list;
}

/**
 * Get remote name of branch
 *
 * @param branch S4 class git_branch
 * @return character string with remote name.
 */
SEXP git2r_branch_remote_name(SEXP branch)
{
    int err;
    SEXP result = R_NilValue;
    const char *name;
    git_buf buf = {0};
    git_branch_t type;
    git_reference *reference = NULL;
    git_repository *repository = NULL;

    if (git2r_error_check_branch_arg(branch))
        error("Invalid arguments to git2r_branch_remote_name");

    type = INTEGER(GET_SLOT(branch, Rf_install("type")))[0];
    if (GIT_BRANCH_REMOTE != type)
        error("branch is not remote");

    repository = git2r_repository_open(GET_SLOT(branch, Rf_install("repo")));
    if (!repository)
        error(git2r_err_invalid_repository);

    name = CHAR(STRING_ELT(GET_SLOT(branch, Rf_install("name")), 0));
    err = git_branch_lookup(&reference, repository, name, type);
    if (err < 0)
        goto cleanup;

    err = git_branch_remote_name(&buf,
                                 repository,
                                 git_reference_name(reference));
    if (err < 0)
        goto cleanup;

    PROTECT(result = allocVector(STRSXP, 1));
    SET_STRING_ELT(result, 0, mkChar(buf.ptr));
    git_buf_free(&buf);

cleanup:
    if (reference)
        git_reference_free(reference);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return result;
}

/**
 * Get remote url of branch
 *
 * @param branch S4 class git_branch
 * @return character string with remote url.
 */
SEXP git2r_branch_remote_url(SEXP branch)
{
    int err;
    SEXP result = R_NilValue;
    const char *name;
    git_buf buf = {0};
    git_branch_t type;
    git_reference *reference = NULL;
    git_remote *remote = NULL;
    git_repository *repository = NULL;

    if (git2r_error_check_branch_arg(branch))
        error("Invalid arguments to git2r_branch_remote_url");

    type = INTEGER(GET_SLOT(branch, Rf_install("type")))[0];
    if (GIT_BRANCH_REMOTE != type)
        error("branch is not remote");

    repository = git2r_repository_open(GET_SLOT(branch, Rf_install("repo")));
    if (!repository)
        error(git2r_err_invalid_repository);

    name = CHAR(STRING_ELT(GET_SLOT(branch, Rf_install("name")), 0));
    err = git_branch_lookup(&reference, repository, name, type);
    if (err < 0)
        goto cleanup;

    err = git_branch_remote_name(&buf,
                                 repository,
                                 git_reference_name(reference));
    if (err < 0)
        goto cleanup;

    err = git_remote_load(&remote, repository, buf.ptr);
    if (err < 0) {
        err = git_remote_create_anonymous(&remote, repository, buf.ptr, NULL);
        if (err < 0) {
            git_buf_free(&buf);
            goto cleanup;
        }
    }
    git_buf_free(&buf);

    PROTECT(result = allocVector(STRSXP, 1));
    SET_STRING_ELT(result, 0, mkChar(git_remote_url(remote)));

cleanup:
    if (remote)
        git_remote_free(remote);

    if (reference)
        git_reference_free(reference);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return result;
}

/**
 * Rename a branch
 *
 * @param branch Branch to rename
 * @param new_branch_name The new name for the branch
 * @param force Overwrite existing branch
 * @param signature The identity that will be used to populate the reflog entry
 * @param message The one line long message to the reflog. If NULL, the default
 *        value is appended
 * @return The renamed S4 class git_branch
 */
SEXP git2r_branch_rename(
    SEXP branch,
    SEXP new_branch_name,
    SEXP force,
    SEXP signature,
    SEXP message)
{
    SEXP repo;
    SEXP result = R_NilValue;
    int err;
    int overwrite = 0;
    const char *log = NULL;
    const char *name = NULL;
    git_branch_t type;
    git_signature *who = NULL;
    git_reference *reference = NULL;
    git_reference *new_reference = NULL;
    git_repository *repository = NULL;

    if (git2r_error_check_branch_arg(branch)
        || git2r_error_check_string_arg(new_branch_name)
        || git2r_error_check_logical_arg(force)
        || git2r_error_check_signature_arg(signature))
        error("Invalid arguments to git2r_branch_rename");

    if (R_NilValue != message) {
        if (git2r_error_check_string_arg(message))
            error("Invalid arguments to git2r_branch_rename");
        log = CHAR(STRING_ELT(message, 0));
    }

    repo = GET_SLOT(branch, Rf_install("repo"));
    repository = git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    type = INTEGER(GET_SLOT(branch, Rf_install("type")))[0];
    name = CHAR(STRING_ELT(GET_SLOT(branch, Rf_install("name")), 0));
    err = git_branch_lookup(&reference, repository, name, type);
    if (err < 0)
        goto cleanup;

    if (LOGICAL(force)[0])
        overwrite = 1;

    err = git2r_signature_from_arg(&who, signature);
    if (err < 0)
        goto cleanup;

    err = git_branch_move(
        &new_reference,
        reference,
        CHAR(STRING_ELT(new_branch_name, 0)),
        overwrite,
        who,
        log);
    if (err)
        goto cleanup;

    PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_branch")));
    err = git2r_branch_init(new_reference, type, repo, result);

cleanup:
    if (reference)
        git_reference_free(reference);

    if (new_reference)
        git_reference_free(new_reference);

    if (who)
        git_signature_free(who);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return result;
}

/**
 * Get hex pointed to by a branch
 *
 * @param branch S4 class git_branch
 * @return 40 character hex value if the reference is direct, else NA
 */
SEXP git2r_branch_target(SEXP branch)
{
    int err;
    SEXP result = R_NilValue;
    const char *name;
    char hex[GIT_OID_HEXSZ + 1];
    git_branch_t type;
    git_reference *reference = NULL;
    git_repository *repository = NULL;

    if (git2r_error_check_branch_arg(branch))
        error("Invalid arguments to git2r_branch_target");

    repository = git2r_repository_open(GET_SLOT(branch, Rf_install("repo")));
    if (!repository)
        error(git2r_err_invalid_repository);

    name = CHAR(STRING_ELT(GET_SLOT(branch, Rf_install("name")), 0));
    type = INTEGER(GET_SLOT(branch, Rf_install("type")))[0];
    err = git_branch_lookup(&reference, repository, name, type);
    if (err < 0)
        goto cleanup;

    PROTECT(result = allocVector(STRSXP, 1));
    if (GIT_REF_OID == git_reference_type(reference)) {
        git_oid_fmt(hex, git_reference_target(reference));
        hex[GIT_OID_HEXSZ] = '\0';
        SET_STRING_ELT(result, 0, mkChar(hex));
    } else {
        SET_STRING_ELT(result, 0, NA_STRING);
    }

cleanup:
    if (reference)
        git_reference_free(reference);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return result;
}

/**
 * Get remote tracking branch, given a local branch.
 *
 * @param branch S4 class git_branch
 * @return S4 class git_branch or R_NilValue if no remote tracking branch.
 */
SEXP git2r_branch_get_upstream(SEXP branch)
{
    int err;
    SEXP result = R_NilValue;
    SEXP repo;
    const char *name;
    git_branch_t type;
    git_reference *reference = NULL;
    git_reference *upstream = NULL;
    git_repository *repository = NULL;

    if (git2r_error_check_branch_arg(branch))
        error("Invalid arguments to git2r_branch_get_upstream");

    repo = GET_SLOT(branch, Rf_install("repo"));
    repository = git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    name = CHAR(STRING_ELT(GET_SLOT(branch, Rf_install("name")), 0));
    type = INTEGER(GET_SLOT(branch, Rf_install("type")))[0];
    err = git_branch_lookup(&reference, repository, name, type);
    if (err < 0)
        goto cleanup;

    err = git_branch_upstream(&upstream, reference);
    if (err < 0) {
        if (GIT_ENOTFOUND == err)
            err = 0;
        goto cleanup;
    }

    PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_branch")));
    err = git2r_branch_init(upstream, GIT_BRANCH_REMOTE, repo, result);

cleanup:
    if (reference)
        git_reference_free(reference);

    if (upstream)
        git_reference_free(upstream);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return result;
}

/**
 * Set remote tracking branch
 *
 * Set the upstream configuration for a given local branch
 * @param branch The branch to configure
 * @param upstream_name remote-tracking or local branch to set as
 * upstream. Pass NULL to unset.
 * @return R_NilValue
 */
SEXP git2r_branch_set_upstream(SEXP branch, SEXP upstream_name)
{
    int err;
    SEXP repo;
    const char *name;
    const char *u_name = NULL;
    git_branch_t type;
    git_reference *reference = NULL;
    git_repository *repository = NULL;

    if (git2r_error_check_branch_arg(branch))
        error("Invalid arguments to git2r_branch_set_upstream");

    if (R_NilValue != upstream_name) {
        if (git2r_error_check_string_arg(upstream_name))
            error("Invalid arguments to git2r_branch_set_upstream");
        u_name = CHAR(STRING_ELT(upstream_name, 0));
    }

    repo = GET_SLOT(branch, Rf_install("repo"));
    repository = git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    name = CHAR(STRING_ELT(GET_SLOT(branch, Rf_install("name")), 0));
    type = INTEGER(GET_SLOT(branch, Rf_install("type")))[0];
    err = git_branch_lookup(&reference, repository, name, type);
    if (err < 0)
        goto cleanup;

    err = git_branch_set_upstream(&reference, u_name);

cleanup:
    if (reference)
        git_reference_free(reference);

    if (repository)
        git_repository_free(repository);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return R_NilValue;
}
