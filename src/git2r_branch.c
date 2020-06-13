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
#include "git2r_branch.h"
#include "git2r_commit.h"
#include "git2r_deprecated.h"
#include "git2r_error.h"
#include "git2r_reference.h"
#include "git2r_repository.h"
#include "git2r_S3.h"
#include "git2r_signature.h"

/**
 * Count number of branches.
 *
 * @param repo S3 class git_repository
 * @param flags Filtering flags for the branch listing. Valid values
 *        are 1 (LOCAL), 2 (REMOTE) and 3 (ALL)
 * @param n The number of branches
 * @return 0 on success, or an error code.
 */
static int
git2r_branch_count(
    git_repository *repo,
    int flags,
    size_t *n)
{
    int error;
    git_branch_iterator *iter;
    git_branch_t type;
    git_reference *ref;

    *n = 0;

    error = git_branch_iterator_new(&iter, repo, flags);
    if (error)
        return error;

    for (;;) {
        error = git_branch_next(&ref, &type, iter);
        if (error)
            break;
        git_reference_free(ref);
        (*n)++;
    }

    git_branch_iterator_free(iter);

    if (GIT_ITEROVER != error)
        return error;
    return 0;
}

/**
 * Init slots in S3 class git_branch
 *
 * @param source a reference
 * @param repository the repository
 * @param type the branch type; local or remote
 * @param repo S3 class git_repository that contains the blob
 * @param dest S3 class git_branch to initialize
 * @return int; < 0 if error, else 0
 */
int attribute_hidden
git2r_branch_init(
    const git_reference *source,
    git_branch_t type,
    SEXP repo,
    SEXP dest)
{
    int error;
    const char *name;

    error = git_branch_name(&name, source);
    if (error)
        goto cleanup;
    SET_VECTOR_ELT(dest, git2r_S3_item__git_branch__name, Rf_mkString(name));
    SET_VECTOR_ELT(dest, git2r_S3_item__git_branch__type, Rf_ScalarInteger(type));
    SET_VECTOR_ELT(dest, git2r_S3_item__git_branch__repo, Rf_duplicate(repo));

cleanup:
    return error;
}

/**
 * Create a new branch
 *
 * @param branch_name Name for the branch
 * @param commit Commit to which branch should point.
 * @param force Overwrite existing branch
 * @return S3 class git_branch
 */
SEXP attribute_hidden
git2r_branch_create(
    SEXP branch_name,
    SEXP commit,
    SEXP force)
{
    SEXP repo, result = R_NilValue;
    int error, nprotect = 0, overwrite = 0;
    git_commit *target = NULL;
    git_reference *reference = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_string(branch_name))
        git2r_error(__func__, NULL, "'branch_name'", git2r_err_string_arg);
    if (git2r_arg_check_commit(commit))
        git2r_error(__func__, NULL, "'commit'", git2r_err_commit_arg);
    if (git2r_arg_check_logical(force))
        git2r_error(__func__, NULL, "'force'", git2r_err_logical_arg);

    repo = git2r_get_list_element(commit, "repo");
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git2r_commit_lookup(&target, repository, commit);
    if (error)
        goto cleanup;

    if (LOGICAL(force)[0])
        overwrite = 1;

    error = git_branch_create(
        &reference,
        repository,
        CHAR(STRING_ELT(branch_name, 0)),
        target,
        overwrite);
    if (error)
        goto cleanup;

    PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_branch));
    nprotect++;
    Rf_setAttrib(result, R_ClassSymbol,
                 Rf_mkString(git2r_S3_class__git_branch));
    error = git2r_branch_init(reference, GIT_BRANCH_LOCAL, repo, result);

cleanup:
    git_reference_free(reference);
    git_commit_free(target);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Delete branch
 *
 * @param branch S3 class git_branch
 * @return R_NilValue
 */
SEXP attribute_hidden
git2r_branch_delete(
    SEXP branch)
{
    int error;
    const char *name;
    git_branch_t type;
    git_reference *reference = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_branch(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_branch_arg);

    repository = git2r_repository_open(git2r_get_list_element(branch, "repo"));
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    name = CHAR(STRING_ELT(git2r_get_list_element(branch, "name"), 0));
    type = INTEGER(git2r_get_list_element(branch, "type"))[0];
    error = git_branch_lookup(&reference, repository, name, type);
    if (error)
        goto cleanup;

    error = git_branch_delete(reference);

cleanup:
    git_reference_free(reference);
    git_repository_free(repository);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return R_NilValue;
}

/**
 * Determine if the current local branch is pointed at by HEAD
 *
 * @param branch S3 class git_branch
 * @return TRUE if head, FALSE if not
 */
SEXP attribute_hidden
git2r_branch_is_head(
    SEXP branch)
{
    SEXP result = R_NilValue;
    int error, nprotect = 0;
    const char *name;
    git_reference *reference = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_branch(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_branch_arg);

    repository = git2r_repository_open(git2r_get_list_element(branch, "repo"));
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    name = CHAR(STRING_ELT(git2r_get_list_element(branch, "name"), 0));

    error = git_branch_lookup(
        &reference,
        repository,
        name,
        INTEGER(git2r_get_list_element(branch, "type"))[0]);
    if (error)
        goto cleanup;

    error = git_branch_is_head(reference);
    if (0 == error || 1 == error) {
        PROTECT(result = Rf_allocVector(LGLSXP, 1));
        nprotect++;
        LOGICAL(result)[0] = error;
        error = 0;
    }

cleanup:
    git_reference_free(reference);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * List branches in a repository
 *
 * @param repo S3 class git_repository
 * @param flags Filtering flags for the branch listing. Valid values
 *        are 1 (LOCAL), 2 (REMOTE) and 3 (ALL)
 * @return VECXSP with S3 objects of class git_branch
 */
SEXP attribute_hidden
git2r_branch_list(
    SEXP repo,
    SEXP flags)
{
    SEXP names, result = R_NilValue;
    int error, nprotect = 0;
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
    error = git2r_branch_count(repository, INTEGER(flags)[0], &n);
    if (error)
        goto cleanup;
    PROTECT(result = Rf_allocVector(VECSXP, n));
    nprotect++;
    Rf_setAttrib(result, R_NamesSymbol, names = Rf_allocVector(STRSXP, n));

    error = git_branch_iterator_new(&iter, repository,  INTEGER(flags)[0]);
    if (error)
        goto cleanup;

    for (i = 0; i < n; i++) {
        SEXP branch;

        error = git_branch_next(&reference, &type, iter);
        if (error) {
            if (GIT_ITEROVER == error)
                error = GIT_OK;
            goto cleanup;
        }

        SET_VECTOR_ELT(result, i,
                       branch = Rf_mkNamed(VECSXP, git2r_S3_items__git_branch));
        Rf_setAttrib(branch, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_branch));
        error = git2r_branch_init(reference, type, repo, branch);
        if (error)
            goto cleanup;
        SET_STRING_ELT(
            names,
            i,
            STRING_ELT(git2r_get_list_element(branch, "name"), 0));
        if (reference)
            git_reference_free(reference);
        reference = NULL;
    }

cleanup:
    git_branch_iterator_free(iter);
    git_reference_free(reference);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Get the full name of a branch
 *
 * @param branch S3 class git_branch
 * @return character string with full name of branch.
 */
SEXP attribute_hidden
git2r_branch_canonical_name(
    SEXP branch)
{
    int error, nprotect = 0;
    SEXP result = R_NilValue;
    const char *name;
    git_branch_t type;
    git_reference *reference = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_branch(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_branch_arg);

    repository = git2r_repository_open(git2r_get_list_element(branch, "repo"));
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    name = CHAR(STRING_ELT(git2r_get_list_element(branch, "name"), 0));
    type = INTEGER(git2r_get_list_element(branch, "type"))[0];
    error = git_branch_lookup(&reference, repository, name, type);
    if (error)
        goto cleanup;

    PROTECT(result = Rf_allocVector(STRSXP, 1));
    nprotect++;
    SET_STRING_ELT(result, 0, Rf_mkChar(git_reference_name(reference)));

cleanup:
    git_reference_free(reference);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}
/**
 * Get the configured canonical name of the upstream branch, given a
 * local branch, i.e "branch.branch_name.merge" property of the config
 * file.
 *
 * @param branch S3 class git_branch.
 * @return Character vector of length one with upstream canonical name.
 */
SEXP attribute_hidden
git2r_branch_upstream_canonical_name(
    SEXP branch)
{
    int error, nprotect = 0;
    SEXP result = R_NilValue;
    SEXP repo;
    const char *name;
    git_branch_t type;
    const char *branch_name;
    size_t branch_name_len;
    char *buf = NULL;
    size_t buf_len;
    git_config *cfg = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_branch(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_branch_arg);

    type = INTEGER(git2r_get_list_element(branch, "type"))[0];
    if (GIT_BRANCH_LOCAL != type)
        git2r_error(__func__, NULL, git2r_err_branch_not_local, NULL);

    repo = git2r_get_list_element(branch, "repo");
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git_repository_config_snapshot(&cfg, repository);
    if (error)
        goto cleanup;

    branch_name = CHAR(STRING_ELT(git2r_get_list_element(branch, "name"), 0));
    branch_name_len = strlen(branch_name);
    while (branch_name[0] == '.') {
        branch_name++;
        branch_name_len--;
    }
    while (branch_name_len >= 1 && branch_name[branch_name_len - 1] == '.') {
        branch_name_len--;
    }
    buf_len = branch_name_len + sizeof("branch." ".merge");
    buf = malloc(buf_len);
    if (!buf) {
        GIT2R_ERROR_SET_OOM();
        error = GIT2R_ERROR_NOMEMORY;
        goto cleanup;
    }
    error = snprintf(buf, buf_len, "branch.%.*s.merge", (int)branch_name_len, branch_name);
    if (error < 0 || (size_t)error >= buf_len) {
        GIT2R_ERROR_SET_STR(GIT2R_ERROR_OS, "Failed to snprintf branch config.");
        error = GIT2R_ERROR_OS;
        goto cleanup;
    }

    error = git_config_get_string(&name, cfg, buf);
    if (error)
        goto cleanup;

    PROTECT(result = Rf_allocVector(STRSXP, 1));
    nprotect++;
    SET_STRING_ELT(result, 0, Rf_mkChar(name));

cleanup:
    if (buf)
        free(buf);
    git_config_free(cfg);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Get remote name of branch
 *
 * @param branch S3 class git_branch
 * @return character string with remote name.
 */
SEXP attribute_hidden
git2r_branch_remote_name(
    SEXP branch)
{
    int error, nprotect = 0;
    SEXP result = R_NilValue;
    const char *name;
    git_buf buf = {0};
    git_branch_t type;
    git_reference *reference = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_branch(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_branch_arg);

    type = INTEGER(git2r_get_list_element(branch, "type"))[0];
    if (GIT_BRANCH_REMOTE != type)
        git2r_error(__func__, NULL, git2r_err_branch_not_remote, NULL);

    repository = git2r_repository_open(git2r_get_list_element(branch, "repo"));
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    name = CHAR(STRING_ELT(git2r_get_list_element(branch, "name"), 0));
    error = git_branch_lookup(&reference, repository, name, type);
    if (error)
        goto cleanup;

    error = git_branch_remote_name(
        &buf,
        repository,
        git_reference_name(reference));
    if (error)
        goto cleanup;

    PROTECT(result = Rf_allocVector(STRSXP, 1));
    nprotect++;
    SET_STRING_ELT(result, 0, Rf_mkChar(buf.ptr));
    GIT2R_BUF_DISPOSE(&buf);

cleanup:
    git_reference_free(reference);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Get remote url of branch
 *
 * @param branch S3 class git_branch
 * @return character string with remote url.
 */
SEXP attribute_hidden
git2r_branch_remote_url(
    SEXP branch)
{
    int error, nprotect = 0;
    SEXP result = R_NilValue;
    const char *name;
    git_buf buf = {0};
    git_branch_t type;
    git_reference *reference = NULL;
    git_remote *remote = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_branch(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_branch_arg);

    type = INTEGER(git2r_get_list_element(branch, "type"))[0];
    if (GIT_BRANCH_REMOTE != type)
        git2r_error(__func__, NULL, git2r_err_branch_not_remote, NULL);

    repository = git2r_repository_open(git2r_get_list_element(branch, "repo"));
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    name = CHAR(STRING_ELT(git2r_get_list_element(branch, "name"), 0));
    error = git_branch_lookup(&reference, repository, name, type);
    if (error)
        goto cleanup;

    error = git_branch_remote_name(
        &buf,
        repository,
        git_reference_name(reference));
    if (error)
        goto cleanup;

    error = git_remote_lookup(&remote, repository, buf.ptr);
    if (error) {
        error = git_remote_create_anonymous(&remote, repository, buf.ptr);
        if (error) {
            GIT2R_BUF_DISPOSE(&buf);
            goto cleanup;
        }
    }

    GIT2R_BUF_DISPOSE(&buf);

    PROTECT(result = Rf_allocVector(STRSXP, 1));
    nprotect++;
    SET_STRING_ELT(result, 0, Rf_mkChar(git_remote_url(remote)));

cleanup:
    git_remote_free(remote);
    git_reference_free(reference);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Rename a branch
 *
 * @param branch Branch to rename
 * @param new_branch_name The new name for the branch
 * @param force Overwrite existing branch
 * @return The renamed S3 class git_branch
 */
SEXP attribute_hidden
git2r_branch_rename(
    SEXP branch,
    SEXP new_branch_name,
    SEXP force)
{
    SEXP repo, result = R_NilValue;
    int error, nprotect = 0, overwrite = 0;
    const char *name = NULL;
    git_branch_t type;
    git_reference *reference = NULL, *new_reference = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_branch(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_branch_arg);
    if (git2r_arg_check_string(new_branch_name))
        git2r_error(__func__, NULL, "'new_branch_name'", git2r_err_string_arg);
    if (git2r_arg_check_logical(force))
        git2r_error(__func__, NULL, "'force'", git2r_err_logical_arg);

    repo = git2r_get_list_element(branch, "repo");
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    type = INTEGER(git2r_get_list_element(branch, "type"))[0];
    name = CHAR(STRING_ELT(git2r_get_list_element(branch, "name"), 0));
    error = git_branch_lookup(&reference, repository, name, type);
    if (error)
        goto cleanup;

    if (LOGICAL(force)[0])
        overwrite = 1;

    error = git_branch_move(
        &new_reference,
        reference,
        CHAR(STRING_ELT(new_branch_name, 0)),
        overwrite);
    if (error)
        goto cleanup;

    PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_branch));
    nprotect++;
    Rf_setAttrib(result, R_ClassSymbol,
                 Rf_mkString(git2r_S3_class__git_branch));
    error = git2r_branch_init(new_reference, type, repo, result);

cleanup:
    git_reference_free(reference);
    git_reference_free(new_reference);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Get sha pointed to by a branch
 *
 * @param branch S3 class git_branch
 * @return The 40 character sha if the reference is direct, else NA
 */
SEXP attribute_hidden
git2r_branch_target(
    SEXP branch)
{
    int error, nprotect = 0;
    SEXP result = R_NilValue;
    const char *name;
    char sha[GIT_OID_HEXSZ + 1];
    git_branch_t type;
    git_reference *reference = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_branch(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_branch_arg);

    repository = git2r_repository_open(git2r_get_list_element(branch, "repo"));
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    name = CHAR(STRING_ELT(git2r_get_list_element(branch, "name"), 0));
    type = INTEGER(git2r_get_list_element(branch, "type"))[0];
    error = git_branch_lookup(&reference, repository, name, type);
    if (error)
        goto cleanup;

    PROTECT(result = Rf_allocVector(STRSXP, 1));
    nprotect++;
    if (git_reference_type(reference) == GIT2R_REFERENCE_DIRECT) {
        git_oid_fmt(sha, git_reference_target(reference));
        sha[GIT_OID_HEXSZ] = '\0';
        SET_STRING_ELT(result, 0, Rf_mkChar(sha));
    } else {
        SET_STRING_ELT(result, 0, NA_STRING);
    }

cleanup:
    git_reference_free(reference);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Get remote tracking branch, given a local branch.
 *
 * @param branch S3 class git_branch
 * @return S3 class git_branch or R_NilValue if no remote tracking branch.
 */
SEXP attribute_hidden
git2r_branch_get_upstream(
    SEXP branch)
{
    int error, nprotect = 0;
    SEXP repo, result = R_NilValue;
    const char *name;
    git_branch_t type;
    git_reference *reference = NULL, *upstream = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_branch(branch))
        git2r_error(__func__, NULL, "'branch'", git2r_err_branch_arg);

    repo = git2r_get_list_element(branch, "repo");
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    name = CHAR(STRING_ELT(git2r_get_list_element(branch, "name"), 0));
    type = INTEGER(git2r_get_list_element(branch, "type"))[0];
    error = git_branch_lookup(&reference, repository, name, type);
    if (error)
        goto cleanup;

    error = git_branch_upstream(&upstream, reference);
    if (error) {
        if (GIT_ENOTFOUND == error)
            error = GIT_OK;
        goto cleanup;
    }

    PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_branch));
    nprotect++;
    Rf_setAttrib(result, R_ClassSymbol,
                 Rf_mkString(git2r_S3_class__git_branch));
    error = git2r_branch_init(upstream, GIT_BRANCH_REMOTE, repo, result);

cleanup:
    git_reference_free(reference);
    git_reference_free(upstream);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

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
SEXP attribute_hidden
git2r_branch_set_upstream(
    SEXP branch,
    SEXP upstream_name)
{
    int error;
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

    repo = git2r_get_list_element(branch, "repo");
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    name = CHAR(STRING_ELT(git2r_get_list_element(branch, "name"), 0));
    type = INTEGER(git2r_get_list_element(branch, "type"))[0];
    error = git_branch_lookup(&reference, repository, name, type);
    if (error)
        goto cleanup;

    error = git_branch_set_upstream(reference, u_name);

cleanup:
    git_reference_free(reference);
    git_repository_free(repository);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return R_NilValue;
}
