/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2019 The git2r contributors
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
#include "git2r_commit.h"
#include "git2r_deprecated.h"
#include "git2r_error.h"
#include "git2r_oid.h"
#include "git2r_repository.h"
#include "git2r_S3.h"
#include "git2r_signature.h"
#include "git2r_tree.h"

/**
 * Check for any changes in index
 *
 * @param repository The repository
 * @return 0 if ok, else error code.
 */
static int
git2r_any_changes_in_index(
    git_repository *repository)
{
    int error;
    int changes_in_index = 0;
    size_t i, count;
    git_status_list *status = NULL;
    git_status_options opts = GIT_STATUS_OPTIONS_INIT;
    opts.show  = GIT_STATUS_SHOW_INDEX_ONLY;

    error = git_status_list_new(&status, repository, &opts);
    if (error)
        goto cleanup;

    count = git_status_list_entrycount(status);
    for (i = 0; i < count; ++i) {
        const git_status_entry *s = git_status_byindex(status, i);

        if (s->status == GIT_STATUS_CURRENT)
            continue;

        if (s->status & GIT_STATUS_INDEX_NEW)
            changes_in_index = 1;
        else if (s->status & GIT_STATUS_INDEX_MODIFIED)
            changes_in_index = 1;
        else if (s->status & GIT_STATUS_INDEX_DELETED)
            changes_in_index = 1;
        else if (s->status & GIT_STATUS_INDEX_RENAMED)
            changes_in_index = 1;
        else if (s->status & GIT_STATUS_INDEX_TYPECHANGE)
            changes_in_index = 1;

        if (changes_in_index)
            break;
    }

    if (!changes_in_index) {
        GIT2R_ERROR_SET_STR(GIT2R_ERROR_NONE, git2r_err_nothing_added_to_commit);
        error = GIT_ERROR;
    }

cleanup:
    git_status_list_free(status);
    return error;
}

/**
 * Close the commits in parents and free memory of parents.
 *
 * @param parents The parent vector of commits.
 * @param n_parents The number of parents.
 * @return void
 */
static void
git2r_parents_free(
    git_commit **parents,
    size_t n_parents)
{
    if (parents) {
        size_t i;

        for (i = 0; i < n_parents; i++) {
            if (parents[i])
                git_commit_free(parents[i]);
        }

        free(parents);
    }
}

/**
 * Data structure to hold information when iterating over MERGE_HEAD
 * entries.
 */
typedef struct {
    size_t n;
    git_repository *repository;
    git_commit **parents;
} git2r_merge_head_cb_data;

/**
 * Invoked 'callback' for each ID in the MERGE_HEAD file.
 *
 * @param oid The id of the merge head
 * @param payload Payload data passed to 'git_repository_mergehead_foreach'
 * @return 0
 */
static int
git2r_repository_mergehead_foreach_cb(
    const git_oid *oid,
    void *payload)
{
    int error = 0;
    git2r_merge_head_cb_data *cb_data = (git2r_merge_head_cb_data*)payload;

    if (cb_data->parents)
        error = git_commit_lookup(
            &(cb_data->parents[cb_data->n]),
            cb_data->repository,
            oid);

    cb_data->n += 1;

    return error;
}

/**
 * Retrieve parents of the commit under construction
 *
 * @param parents The vector of parents to create and populate.
 * @param n_parents The length of parents vector
 * @param repository The repository
 * @return 0 on succes, or error code
 */
static int
git2r_retrieve_parents(
    git_commit ***parents,
    size_t *n_parents,
    git_repository *repository)
{
    int error;
    git_oid oid;
    git2r_merge_head_cb_data cb_data = {0, NULL, NULL};
    git_repository_state_t state;

    error = git_repository_head_unborn(repository);
    if (1 == error) {
        *n_parents = 0;
        return GIT_OK;
    } else if (0 != error) {
        return error;
    }

    state = git_repository_state(repository);
    if (state == GIT_REPOSITORY_STATE_MERGE) {
        /* Count number of merge heads */
        error = git_repository_mergehead_foreach(
            repository,
            git2r_repository_mergehead_foreach_cb,
            &cb_data);
        if (error)
            return error;
    }

    *parents = calloc(cb_data.n + 1, sizeof(git_commit*));
    if (!parents) {
        GIT2R_ERROR_SET_STR(GIT2R_ERROR_NONE, git2r_err_alloc_memory_buffer);
        return GIT_ERROR;
    }
    *n_parents = cb_data.n + 1;

    error = git_reference_name_to_id(&oid, repository, "HEAD");
    if (error)
        return error;

    error = git_commit_lookup(&**parents, repository, &oid);
    if (error)
        return error;

    if (state == GIT_REPOSITORY_STATE_MERGE) {
        /* Append merge heads to parents */
        cb_data.n = 0;
        cb_data.repository = repository;
        cb_data.parents = *parents + 1;
        error = git_repository_mergehead_foreach(
            repository,
            git2r_repository_mergehead_foreach_cb,
            &cb_data);
        if (error)
            return error;
    }

    return GIT_OK;
}

/**
 * Create a commit
 *
 * @param out The oid of the newly created commit
 * @param repository The repository
 * @param index The index
 * @param message The commit message
 * @param author Who is the author of the commit
 * @param committer Who is the committer
 * @return 0 on success, or error code
 */
int attribute_hidden
git2r_commit_create(
    git_oid *out,
    git_repository *repository,
    git_index *index,
    const char *message,
    git_signature *author,
    git_signature *committer)
{
    int error;
    git_oid oid;
    git_tree *tree = NULL;
    git_commit **parents = NULL;
    size_t n_parents = 0;

    error = git_index_write_tree(&oid, index);
    if (error)
        goto cleanup;

    error = git_tree_lookup(&tree, repository, &oid);
    if (error)
        goto cleanup;

    error = git2r_retrieve_parents(&parents, &n_parents, repository);
    if (error)
        goto cleanup;

    error = git_commit_create(
        out,
        repository,
        "HEAD",
        author,
        committer,
        NULL,
        message,
        tree,
        n_parents,
        (const git_commit**)parents);
    if (error)
        goto cleanup;

    error = git_repository_state_cleanup(repository);

cleanup:
    git2r_parents_free(parents, n_parents);
    git_tree_free(tree);
    return error;
}

/**
 * Commit
 *
 * @param repo S3 class git_repository
 * @param message The message for the commit
 * @param author S3 class git_signature
 * @param committer S3 class git_signature
 * @return S3 class git_commit
 */
SEXP attribute_hidden
git2r_commit(
    SEXP repo,
    SEXP message,
    SEXP author,
    SEXP committer)
{
    int error, nprotect = 0;
    SEXP result = R_NilValue;
    git_signature *c_author = NULL;
    git_signature *c_committer = NULL;
    git_index *index = NULL;
    git_oid oid;
    git_repository *repository = NULL;
    git_commit *commit = NULL;

    if (git2r_arg_check_string(message))
        git2r_error(__func__, NULL, "'message'", git2r_err_string_arg);
    if (git2r_arg_check_signature(author))
        git2r_error(__func__, NULL, "'author'", git2r_err_signature_arg);
    if (git2r_arg_check_signature(committer))
        git2r_error(__func__, NULL, "'committer'", git2r_err_signature_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git2r_signature_from_arg(&c_author, author);
    if (error)
        goto cleanup;

    error = git2r_signature_from_arg(&c_committer, committer);
    if (error)
        goto cleanup;

    error = git2r_any_changes_in_index(repository);
    if (error)
        goto cleanup;

    error = git_repository_index(&index, repository);
    if (error)
        goto cleanup;

    error = git2r_commit_create(
        &oid,
        repository,
        index,
        CHAR(STRING_ELT(message, 0)),
        c_author,
        c_committer);
    if (error)
        goto cleanup;

    error = git_commit_lookup(&commit, repository, &oid);
    if (error)
        goto cleanup;

    PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_commit));
    nprotect++;
    Rf_setAttrib(result, R_ClassSymbol,
                 Rf_mkString(git2r_S3_class__git_commit));
    git2r_commit_init(commit, repo, result);

cleanup:
    git_signature_free(c_author);
    git_signature_free(c_committer);
    git_index_free(index);
    git_repository_free(repository);
    git_commit_free(commit);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Get commit object from S3 class git_commit
 *
 * @param out Pointer to the looked up commit
 * @param repository The repository
 * @param commit S3 class git_commit
 * @return 0 or an error code
 */
int attribute_hidden
git2r_commit_lookup(
    git_commit **out,
    git_repository *repository,
    SEXP commit)
{
    SEXP sha;
    git_oid oid;

    sha = git2r_get_list_element(commit, "sha");
    git_oid_fromstr(&oid, CHAR(STRING_ELT(sha, 0)));
    return git_commit_lookup(out, repository, &oid);
}

/**
 * Get the tree pointed to by a commit
 *
 * @param commit S3 class git_commit or git_stash
 * @return S3 class git_tree
 */
SEXP attribute_hidden
git2r_commit_tree(
    SEXP commit)
{
    int error, nprotect = 0;
    SEXP result = R_NilValue;
    SEXP repo;
    git_commit *commit_obj = NULL;
    git_repository *repository = NULL;
    git_tree *tree = NULL;

    if (git2r_arg_check_commit_stash(commit))
        git2r_error(__func__, NULL, "'commit'", git2r_err_commit_stash_arg);

    repo = git2r_get_list_element(commit, "repo");
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git2r_commit_lookup(&commit_obj, repository, commit);
    if (error)
        goto cleanup;

    error = git_commit_tree(&tree, commit_obj);
    if (error)
        goto cleanup;

    PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_tree));
    nprotect++;
    Rf_setAttrib(result, R_ClassSymbol,
                 Rf_mkString(git2r_S3_class__git_tree));
    git2r_tree_init((git_tree*)tree, repo, result);

cleanup:
    git_commit_free(commit_obj);
    git_tree_free(tree);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Init slots in S3 class git_commit
 *
 * @param source a commit object
 * @param repo S3 class git_repository that contains the blob
 * @param dest S3 class git_commit to initialize
 * @return void
 */
void attribute_hidden
git2r_commit_init(
    git_commit *source,
    SEXP repo,
    SEXP dest)
{
    const char *str;
    const git_signature *signature;
    char sha[GIT_OID_HEXSZ + 1];

    git_oid_fmt(sha, git_commit_id(source));
    sha[GIT_OID_HEXSZ] = '\0';
    SET_VECTOR_ELT(dest, git2r_S3_item__git_commit__sha, Rf_mkString(sha));

    signature = git_commit_author(source);
    if (signature) {
        SEXP elem = VECTOR_ELT(dest, git2r_S3_item__git_commit__author);

        if (Rf_isNull(elem)) {
            SET_VECTOR_ELT(
                dest,
                git2r_S3_item__git_commit__author,
                elem = Rf_mkNamed(VECSXP, git2r_S3_items__git_signature));
            Rf_setAttrib(elem, R_ClassSymbol,
                         Rf_mkString(git2r_S3_class__git_signature));
        }

        git2r_signature_init(signature, elem);
    }

    signature = git_commit_committer(source);
    if (signature) {
        SEXP elem = VECTOR_ELT(dest, git2r_S3_item__git_commit__committer);

        if (Rf_isNull(elem)) {
            SET_VECTOR_ELT(
                dest,
                git2r_S3_item__git_commit__author,
                elem = Rf_mkNamed(VECSXP, git2r_S3_items__git_signature));
            Rf_setAttrib(elem, R_ClassSymbol,
                         Rf_mkString(git2r_S3_class__git_signature));
        }

        git2r_signature_init(signature, elem);
    }

    str = git_commit_summary(source);
    if (str)
        SET_VECTOR_ELT(dest, git2r_S3_item__git_commit__summary, Rf_mkString(str));

    str = git_commit_message(source);
    if (str)
        SET_VECTOR_ELT(dest, git2r_S3_item__git_commit__message, Rf_mkString(str));

    SET_VECTOR_ELT(dest, git2r_S3_item__git_commit__repo, Rf_duplicate(repo));
}

/**
 * Parents of a commit
 *
 * @param commit S3 class git_commit
 * @return list of S3 class git_commit objects
 */
SEXP attribute_hidden
git2r_commit_parent_list(
    SEXP commit)
{
    int error, nprotect = 0;
    size_t i, n;
    SEXP repo;
    SEXP list = R_NilValue;
    git_commit *commit_obj = NULL;
    git_repository *repository = NULL;

    if (git2r_arg_check_commit(commit))
        git2r_error(__func__, NULL, "'commit'", git2r_err_commit_arg);

    repo = git2r_get_list_element(commit, "repo");
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git2r_commit_lookup(&commit_obj, repository, commit);
    if (error)
        goto cleanup;

    n = git_commit_parentcount(commit_obj);
    PROTECT(list = Rf_allocVector(VECSXP, n));
    nprotect++;

    for (i = 0; i < n; i++) {
        git_commit *parent = NULL;
        SEXP item;

        error = git_commit_parent(&parent, commit_obj, i);
        if (error)
            goto cleanup;

        SET_VECTOR_ELT(
            list,
            i,
            item = Rf_mkNamed(VECSXP, git2r_S3_items__git_commit));
        Rf_setAttrib(item, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_commit));
        git2r_commit_init(parent, repo, item);
        git_commit_free(parent);
    }

cleanup:
    git_commit_free(commit_obj);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return list;
}
