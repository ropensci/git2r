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
#include "refs.h"

#include "git2r_arg.h"
#include "git2r_branch.h"
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
 * @param n The number of branches
 * @return 0 on success, or an error code.
 */
static int git2r_branch_count(git_repository *repo, int flags, size_t *n)
{
    int err;
    git_branch_iterator *iter;
    git_branch_t type;
    git_reference *ref;

    *n = 0;

    err = git_branch_iterator_new(&iter, repo, flags);
    if (err)
        return err;

    for (;;) {
        err = git_branch_next(&ref, &type, iter);
        if (err)
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
int git2r_branch_init(
    const git_reference *source,
    git_branch_t type,
    SEXP repo,
    SEXP dest)
{
    int err;
    const char *name;
    SEXP s_name = Rf_install("name");
    SEXP s_type = Rf_install("type");
    SEXP s_repo = Rf_install("repo");

    err = git_branch_name(&name, source);
    if (err)
        goto cleanup;
    SET_SLOT(dest, s_name, Rf_mkString(name));
    SET_SLOT(dest, s_type, Rf_ScalarInteger(type));
    SET_SLOT(dest, s_repo, repo);

cleanup:
    return err;
}

/**
 * Create a new branch
 *
 * @param branch_name Name for the branch
 * @param commit Commit to which branch should point.
 * @param force Overwrite existing branch
 * @return S4 class git_branch
 */
SEXP git2r_branch_create(
    SEXP branch_name,
    SEXP commit,
    SEXP force)
{
    SEXP repo;
    SEXP result = R_NilValue;
    int err;
    int overwrite = 0;
    git_commit *target = NULL;
    git_reference *reference = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_string(branch_name))
        git2r_error(__func__, NULL, "'branch_name'", git2r_err_string_arg);
    if (git2r_arg_check_commit(commit))
        git2r_error(__func__, NULL, "'commit'", git2r_err_commit_arg);
    if (git2r_arg_check_logical(force))
        git2r_error(__func__, NULL, "'force'", git2r_err_logical_arg);

    repo = GET_SLOT(commit, Rf_install("repo"));
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    err = git2r_commit_lookup(&target, repository, commit);
    if (err)
        goto cleanup;

    if (LOGICAL(force)[0])
        overwrite = 1;

    err = git_branch_create(
        &reference,
        repository,
        CHAR(STRING_ELT(branch_name, 0)),
        target,
        overwrite);
    if (err)
        goto cleanup;

    PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_branch")));
    err = git2r_branch_init(reference, GIT_BRANCH_LOCAL, repo, result);

cleanup:
    if (reference)
        git_reference_free(reference);

    if (target)
        git_commit_free(target);

    if (repository)
        git_repository_free(repository);

    if (!Rf_isNull(result))
        UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

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

    if (git2r_arg_check_branch(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_branch_arg);

    repository = git2r_repository_open(GET_SLOT(branch, Rf_install("repo")));
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    name = CHAR(STRING_ELT(GET_SLOT(branch, Rf_install("name")), 0));
    type = INTEGER(GET_SLOT(branch, Rf_install("type")))[0];
    err = git_branch_lookup(&reference, repository, name, type);
    if (err)
        goto cleanup;

    err = git_branch_delete(reference);

cleanup:
    if (reference)
        git_reference_free(reference);

    if (repository)
        git_repository_free(repository);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

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

    if (git2r_arg_check_branch(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_branch_arg);

    repository = git2r_repository_open(GET_SLOT(branch, Rf_install("repo")));
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    name = CHAR(STRING_ELT(GET_SLOT(branch, Rf_install("name")), 0));

    err = git_branch_lookup(
        &reference,
        repository,
        name,
        INTEGER(GET_SLOT(branch, Rf_install("type")))[0]);
    if (err)
        goto cleanup;

    err = git_branch_is_head(reference);
    if (0 == err || 1 == err) {
        PROTECT(result = Rf_allocVector(LGLSXP, 1));
        LOGICAL(result)[0] = err;
        err = GIT_OK;
    }

cleanup:
    if (reference)
        git_reference_free(reference);

    if (repository)
        git_repository_free(repository);

    if (!Rf_isNull(result))
        UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

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
    SEXP result = R_NilValue;
    SEXP names;
    int err;
    git_branch_iterator *iter = NULL;
    size_t i, n = 0;
    git_repository *repository = NULL;
    git_reference *reference = NULL;
    git_branch_t type;

    if (git2r_arg_check_integer(flags))
        git2r_error(__func__, NULL, "'flags'", git2r_err_integer_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    /* Count number of branches before creating the list */
    err = git2r_branch_count(repository, INTEGER(flags)[0], &n);
    if (err)
        goto cleanup;
    PROTECT(result = Rf_allocVector(VECSXP, n));
    setAttrib(result, R_NamesSymbol, names = Rf_allocVector(STRSXP, n));

    err = git_branch_iterator_new(&iter, repository,  INTEGER(flags)[0]);
    if (err)
        goto cleanup;

    for (i = 0; i < n; i++) {
        SEXP branch;

        err = git_branch_next(&reference, &type, iter);
        if (err) {
            if (GIT_ITEROVER == err)
                err = GIT_OK;
            goto cleanup;
        }

        SET_VECTOR_ELT(result, i, branch = NEW_OBJECT(MAKE_CLASS("git_branch")));
        err = git2r_branch_init(reference, type, repo, branch);
        if (err)
            goto cleanup;
        SET_STRING_ELT(
            names,
            i,
            STRING_ELT(GET_SLOT(branch, Rf_install("name")), 0));
        if (reference)
            git_reference_free(reference);
        reference = NULL;
    }

cleanup:
    if (iter)
        git_branch_iterator_free(iter);

    if (reference)
        git_reference_free(reference);

    if (repository)
        git_repository_free(repository);

    if (!Rf_isNull(result))
        UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return result;
}

/**
 * Get the full name of a branch
 *
 * @param branch S4 class git_branch
 * @return character string with full name of branch.
 */
SEXP git2r_branch_canonical_name(SEXP branch)
{
    int err;
    SEXP result = R_NilValue;
    const char *name;
    git_branch_t type;
    git_reference *reference = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_branch(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_branch_arg);

    repository = git2r_repository_open(GET_SLOT(branch, Rf_install("repo")));
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    name = CHAR(STRING_ELT(GET_SLOT(branch, Rf_install("name")), 0));
    type = INTEGER(GET_SLOT(branch, Rf_install("type")))[0];
    err = git_branch_lookup(&reference, repository, name, type);
    if (err)
        goto cleanup;

    PROTECT(result = Rf_allocVector(STRSXP, 1));
    SET_STRING_ELT(result, 0, mkChar(git_reference_name(reference)));

cleanup:
    if (reference)
        git_reference_free(reference);

    if (repository)
        git_repository_free(repository);

    if (!Rf_isNull(result))
        UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return result;
}
/**
 * Get the configured canonical name of the upstream branch, given a
 * local branch, i.e "branch.branch_name.merge" property of the config
 * file.
 *
 * @param branch S4 class git_branch.
 * @return Character vector of length one with upstream canonical name.
 */
SEXP git2r_branch_upstream_canonical_name(SEXP branch)
{
    int err;
    SEXP result = R_NilValue;
    SEXP repo;
    const char *name;
    git_branch_t type;
    git_buf buf = GIT_BUF_INIT;
    git_config *cfg = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_branch(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_branch_arg);

    type = INTEGER(GET_SLOT(branch, Rf_install("type")))[0];
    if (GIT_BRANCH_LOCAL != type)
        git2r_error(__func__, NULL, git2r_err_branch_not_local, NULL);

    repo = GET_SLOT(branch, Rf_install("repo"));
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    err = git_repository_config_snapshot(&cfg, repository);
    if (err)
        goto cleanup;

    err = git_buf_join3(
        &buf,
        '.',
        "branch",
        CHAR(STRING_ELT(GET_SLOT(branch, Rf_install("name")), 0)),
        "merge");
    if (err)
        goto cleanup;

    err = git_config_get_string(&name, cfg, buf.ptr);
    if (err)
        goto cleanup;

    PROTECT(result = Rf_allocVector(STRSXP, 1));
    SET_STRING_ELT(result, 0, mkChar(name));

cleanup:
    git_buf_free(&buf);

    if (cfg)
        git_config_free(cfg);

    if (repository)
        git_repository_free(repository);

    if (!Rf_isNull(result))
        UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return result;
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

    if (git2r_arg_check_branch(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_branch_arg);

    type = INTEGER(GET_SLOT(branch, Rf_install("type")))[0];
    if (GIT_BRANCH_REMOTE != type)
        git2r_error(__func__, NULL, git2r_err_branch_not_remote, NULL);

    repository = git2r_repository_open(GET_SLOT(branch, Rf_install("repo")));
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    name = CHAR(STRING_ELT(GET_SLOT(branch, Rf_install("name")), 0));
    err = git_branch_lookup(&reference, repository, name, type);
    if (err)
        goto cleanup;

    err = git_branch_remote_name(
        &buf,
        repository,
        git_reference_name(reference));
    if (err)
        goto cleanup;

    PROTECT(result = Rf_allocVector(STRSXP, 1));
    SET_STRING_ELT(result, 0, mkChar(buf.ptr));
    git_buf_free(&buf);

cleanup:
    if (reference)
        git_reference_free(reference);

    if (repository)
        git_repository_free(repository);

    if (!Rf_isNull(result))
        UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

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

    if (git2r_arg_check_branch(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_branch_arg);

    type = INTEGER(GET_SLOT(branch, Rf_install("type")))[0];
    if (GIT_BRANCH_REMOTE != type)
        git2r_error(__func__, NULL, git2r_err_branch_not_remote, NULL);

    repository = git2r_repository_open(GET_SLOT(branch, Rf_install("repo")));
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    name = CHAR(STRING_ELT(GET_SLOT(branch, Rf_install("name")), 0));
    err = git_branch_lookup(&reference, repository, name, type);
    if (err)
        goto cleanup;

    err = git_branch_remote_name(
        &buf,
        repository,
        git_reference_name(reference));
    if (err)
        goto cleanup;

    err = git_remote_lookup(&remote, repository, buf.ptr);
    if (err) {
        err = git_remote_create_anonymous(&remote, repository, buf.ptr);
        if (err) {
            git_buf_free(&buf);
            goto cleanup;
        }
    }
    git_buf_free(&buf);

    PROTECT(result = Rf_allocVector(STRSXP, 1));
    SET_STRING_ELT(result, 0, mkChar(git_remote_url(remote)));

cleanup:
    if (remote)
        git_remote_free(remote);

    if (reference)
        git_reference_free(reference);

    if (repository)
        git_repository_free(repository);

    if (!Rf_isNull(result))
        UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return result;
}

/**
 * Rename a branch
 *
 * @param branch Branch to rename
 * @param new_branch_name The new name for the branch
 * @param force Overwrite existing branch
 * @return The renamed S4 class git_branch
 */
SEXP git2r_branch_rename(
    SEXP branch,
    SEXP new_branch_name,
    SEXP force)
{
    SEXP repo;
    SEXP result = R_NilValue;
    int err;
    int overwrite = 0;
    const char *name = NULL;
    git_branch_t type;
    git_reference *reference = NULL;
    git_reference *new_reference = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_branch(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_branch_arg);
    if (git2r_arg_check_string(new_branch_name))
        git2r_error(__func__, NULL, "'new_branch_name'", git2r_err_string_arg);
    if (git2r_arg_check_logical(force))
        git2r_error(__func__, NULL, "'force'", git2r_err_logical_arg);

    repo = GET_SLOT(branch, Rf_install("repo"));
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    type = INTEGER(GET_SLOT(branch, Rf_install("type")))[0];
    name = CHAR(STRING_ELT(GET_SLOT(branch, Rf_install("name")), 0));
    err = git_branch_lookup(&reference, repository, name, type);
    if (err)
        goto cleanup;

    if (LOGICAL(force)[0])
        overwrite = 1;

    err = git_branch_move(
        &new_reference,
        reference,
        CHAR(STRING_ELT(new_branch_name, 0)),
        overwrite);
    if (err)
        goto cleanup;

    PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_branch")));
    err = git2r_branch_init(new_reference, type, repo, result);

cleanup:
    if (reference)
        git_reference_free(reference);

    if (new_reference)
        git_reference_free(new_reference);

    if (repository)
        git_repository_free(repository);

    if (!Rf_isNull(result))
        UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return result;
}

/**
 * Get sha pointed to by a branch
 *
 * @param branch S4 class git_branch
 * @return The 40 character sha if the reference is direct, else NA
 */
SEXP git2r_branch_target(SEXP branch)
{
    int err;
    SEXP result = R_NilValue;
    const char *name;
    char sha[GIT_OID_HEXSZ + 1];
    git_branch_t type;
    git_reference *reference = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_branch(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_branch_arg);

    repository = git2r_repository_open(GET_SLOT(branch, Rf_install("repo")));
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    name = CHAR(STRING_ELT(GET_SLOT(branch, Rf_install("name")), 0));
    type = INTEGER(GET_SLOT(branch, Rf_install("type")))[0];
    err = git_branch_lookup(&reference, repository, name, type);
    if (err)
        goto cleanup;

    PROTECT(result = Rf_allocVector(STRSXP, 1));
    if (GIT_REF_OID == git_reference_type(reference)) {
        git_oid_fmt(sha, git_reference_target(reference));
        sha[GIT_OID_HEXSZ] = '\0';
        SET_STRING_ELT(result, 0, mkChar(sha));
    } else {
        SET_STRING_ELT(result, 0, NA_STRING);
    }

cleanup:
    if (reference)
        git_reference_free(reference);

    if (repository)
        git_repository_free(repository);

    if (!Rf_isNull(result))
        UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

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

    if (git2r_arg_check_branch(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_branch_arg);

    repo = GET_SLOT(branch, Rf_install("repo"));
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    name = CHAR(STRING_ELT(GET_SLOT(branch, Rf_install("name")), 0));
    type = INTEGER(GET_SLOT(branch, Rf_install("type")))[0];
    err = git_branch_lookup(&reference, repository, name, type);
    if (err)
        goto cleanup;

    err = git_branch_upstream(&upstream, reference);
    if (err) {
        if (GIT_ENOTFOUND == err)
            err = GIT_OK;
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

    if (!Rf_isNull(result))
        UNPROTECT(1);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

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

    if (git2r_arg_check_branch(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_branch_arg);
    if (!Rf_isNull(upstream_name)) {
        if (git2r_arg_check_string(upstream_name))
            git2r_error(__func__, NULL, "'upstream_name'", git2r_err_string_arg);
        u_name = CHAR(STRING_ELT(upstream_name, 0));
    }

    repo = GET_SLOT(branch, Rf_install("repo"));
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    name = CHAR(STRING_ELT(GET_SLOT(branch, Rf_install("name")), 0));
    type = INTEGER(GET_SLOT(branch, Rf_install("type")))[0];
    err = git_branch_lookup(&reference, repository, name, type);
    if (err)
        goto cleanup;

    err = git_branch_set_upstream(reference, u_name);

cleanup:
    if (reference)
        git_reference_free(reference);

    if (repository)
        git_repository_free(repository);

    if (err)
        git2r_error(__func__, giterr_last(), NULL, NULL);

    return R_NilValue;
}
