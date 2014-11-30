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
#include "buffer.h"
#include "commit.h"

#include "git2r_arg.h"
#include "git2r_commit.h"
#include "git2r_error.h"
#include "git2r_oid.h"
#include "git2r_repository.h"
#include "git2r_signature.h"
#include "git2r_tree.h"

/**
 * Check for any changes in index
 *
 * @param repository The repository
 * @return 0 if ok, else error code.
 */
static int git2r_any_changes_in_index(git_repository *repository)
{
    int err;
    int changes_in_index = 0;
    size_t i, count;
    git_status_list *status = NULL;
    git_status_options opts = GIT_STATUS_OPTIONS_INIT;
    opts.show  = GIT_STATUS_SHOW_INDEX_ONLY;

    err = git_status_list_new(&status, repository, &opts);
    if (GIT_OK != err)
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
        giterr_set_str(GITERR_NONE, git2r_err_nothing_added_to_commit);
        err = GIT_ERROR;
    }

cleanup:
    if (status)
        git_status_list_free(status);

    return err;
}

/**
 * Create and populate vector of commits.
 *
 * @param out The vector of parents to create and populate.
 * @param repository
 * @param parents The parents as character vector of sha's.
 * @param count The length of parents.
 * @return 0 if ok, else error code.
 */
static int git2r_parents_lookup(
    git_commit ***out,
    git_repository *repository,
    SEXP parents,
    size_t count)
{
    if (count) {
        size_t i = 0;

        *out = calloc(count, sizeof(git_commit*));
        if (NULL == out) {
            giterr_set_str(GITERR_NONE, git2r_err_alloc_memory_buffer);
            return GIT_ERROR;
        }

        for (; i < count; i++) {
            int err;
            git_oid oid;

            err = git_oid_fromstr(&oid, CHAR(STRING_ELT(parents, 0)));
            if (GIT_OK != err)
                return err;

            err = git_commit_lookup((&(*out))[i], repository, &oid);
            if (GIT_OK != err)
                return err;
        }
    }

    return GIT_OK;
}

/**
 * Close the commits in parents and free memory of parents.
 *
 * @param parents The parent vector of commits.
 * @param n_parents The number of parents.
 * @return void
 */
static void git2r_parents_free(git_commit **parents, size_t n_parents)
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
static int git2r_repository_mergehead_foreach_cb(
    const git_oid *oid,
    void *payload)
{
    int err = 0;
    git2r_merge_head_cb_data *cb_data = (git2r_merge_head_cb_data*)payload;

    if (cb_data->parents)
        err = git_commit_lookup(
            &(cb_data->parents[cb_data->n]),
            cb_data->repository,
            oid);

    cb_data->n += 1;

    return err;
}

/**
 * Retrieve parents of the commit under construction
 *
 * @param parents The vector of parents to create and populate.
 * @param n_parents The length of parents vector
 * @param repository The repository
 * @return 0 on succes, or error code
 */
static int git2r_retrieve_parents(
    git_commit ***parents,
    size_t *n_parents,
    git_repository *repository)
{
    int err;
    git_oid oid;
    git2r_merge_head_cb_data cb_data = {0, NULL, NULL};
    git_repository_state_t state = git_repository_state(repository);

    if (state == GIT_REPOSITORY_STATE_MERGE) {
        /* Count number of merge heads */
        err = git_repository_mergehead_foreach(
            repository,
            git2r_repository_mergehead_foreach_cb,
            &cb_data);
        if (GIT_OK != err)
            return err;
    }

    *n_parents = cb_data.n + 1;
    *parents = calloc(*n_parents, sizeof(git_commit*));
    if (!parents) {
        giterr_set_str(GITERR_NONE, git2r_err_alloc_memory_buffer);
        return GIT_ERROR;
    }

    err = git_reference_name_to_id(&oid, repository, "HEAD");
    if (GIT_OK != err)
        return err;

    **parents = malloc(sizeof(git_commit));
    if (!(**parents)) {
        giterr_set_str(GITERR_NONE, git2r_err_alloc_memory_buffer);
        return GIT_ERROR;
    }

    err = git_commit_lookup(&**parents, repository, &oid);
    if (GIT_OK != err)
        return err;

    if (state == GIT_REPOSITORY_STATE_MERGE) {
        /* Append merge heads to parents */
        cb_data.n = 0;
        cb_data.repository = repository;
        cb_data.parents = *parents + 1;
        err = git_repository_mergehead_foreach(
            repository,
            git2r_repository_mergehead_foreach_cb,
            &cb_data);
        if (GIT_OK != err)
            return err;
    }

    return 0;
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
int git2r_commit_create(
    git_oid *out,
    git_repository *repository,
    git_index *index,
    const char *message,
    git_signature *author,
    git_signature *committer)
{
    int err;
    git_oid oid;
    git_tree *tree = NULL;
    git_commit **parents = NULL;
    size_t n_parents = 0;

    err = git_index_write_tree(&oid, index);
    if (GIT_OK != err)
        goto cleanup;

    err = git_tree_lookup(&tree, repository, &oid);
    if (GIT_OK != err)
        goto cleanup;

    err = git2r_retrieve_parents(&parents, &n_parents, repository);
    if (GIT_OK != err)
        goto cleanup;

    err = git_commit_create(
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
    if (GIT_OK != err)
        goto cleanup;

    err = git_repository_state_cleanup(repository);

cleanup:
    if (parents)
        git2r_parents_free(parents, n_parents);

    if (tree)
        git_tree_free(tree);

    return err;
}

/**
 * Commit
 *
 * @param repo S4 class git_repository
 * @param message The message for the commit
 * @param author S4 class git_signature
 * @param committer S4 class git_signature
 * @param parent_list Character vector with sha values of parents
 * @return S4 class git_commit
 */
SEXP git2r_commit(
    SEXP repo,
    SEXP message,
    SEXP author,
    SEXP committer,
    SEXP parent_list)
{
    int err;
    SEXP result = R_NilValue;
    git_signature *c_author = NULL;
    git_signature *c_committer = NULL;
    git_index *index = NULL;
    git_oid commit_id, tree_oid;
    git_repository *repository = NULL;
    git_tree *tree = NULL;
    size_t count = 0;
    git_commit **parents = NULL;
    git_commit *commit = NULL;

    if (GIT_OK != git2r_arg_check_string(message))
        git2r_error(git2r_err_string_arg, __func__, "message");
    if (GIT_OK != git2r_arg_check_signature(author))
        git2r_error(git2r_err_signature_arg, __func__, "author");
    if (GIT_OK != git2r_arg_check_signature(committer))
        git2r_error(git2r_err_signature_arg, __func__, "committer");
    if (GIT_OK != git2r_arg_check_string_vec(parent_list))
        git2r_error(git2r_err_string_vec_arg, __func__, "parent_list");

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git2r_signature_from_arg(&c_author, author);
    if (GIT_OK != err)
        goto cleanup;

    err = git2r_signature_from_arg(&c_committer, committer);
    if (GIT_OK != err)
        goto cleanup;

    err = git2r_any_changes_in_index(repository);
    if (GIT_OK != err)
        goto cleanup;

    err = git_repository_index(&index, repository);
    if (GIT_OK != err)
        goto cleanup;

    if (!git_index_entrycount(index)) {
        giterr_set_str(GITERR_NONE, git2r_err_nothing_added_to_commit);
        err = GIT_ERROR;
        goto cleanup;
    }

    err = git_index_write_tree(&tree_oid, index);
    if (GIT_OK != err)
        goto cleanup;

    err = git_tree_lookup(&tree, repository, &tree_oid);
    if (GIT_OK != err)
        goto cleanup;

    count = LENGTH(parent_list);
    err = git2r_parents_lookup(&parents, repository, parent_list, count);
    if (GIT_OK != err)
        goto cleanup;

    err = git_commit_create(
        &commit_id,
        repository,
        "HEAD",
        c_author,
        c_committer,
        NULL,
        CHAR(STRING_ELT(message, 0)),
        tree,
        count,
        (const git_commit**)parents);
    if (GIT_OK != err)
        goto cleanup;

    err = git_commit_lookup(&commit, repository, &commit_id);
    if (GIT_OK != err)
        goto cleanup;

    PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_commit")));
    git2r_commit_init(commit, repo, result);

cleanup:
    if (c_author)
        git_signature_free(c_author);

    if (c_committer)
        git_signature_free(c_committer);

    if (index)
        git_index_free(index);

    if (tree)
        git_tree_free(tree);

    if (repository)
        git_repository_free(repository);

    if (parents)
        git2r_parents_free(parents, count);

    if (commit)
        git_commit_free(commit);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (GIT_OK != err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return result;
}

/**
 * Get commit object from S4 class git_commit
 *
 * @param out Pointer to the looked up commit
 * @param repository The repository
 * @param commit S4 class git_commit
 * @return 0 or an error code
 */
int git2r_commit_lookup(
    git_commit **out,
    git_repository *repository,
    SEXP commit)
{
    SEXP sha;
    git_oid oid;

    sha = GET_SLOT(commit, Rf_install("sha"));
    git_oid_fromstr(&oid, CHAR(STRING_ELT(sha, 0)));
    return git_commit_lookup(out, repository, &oid);
}

/**
 * Get the tree pointed to by a commit
 *
 * @param commit S4 class git_commit or git_stash
 * @return S4 class git_tree
 */
SEXP git2r_commit_tree(SEXP commit)
{
    int err;
    SEXP result = R_NilValue;
    SEXP repo;
    git_commit *commit_obj = NULL;
    git_repository *repository = NULL;
    git_tree *tree = NULL;

    if (GIT_OK != git2r_arg_check_commit(commit))
        git2r_error(git2r_err_commit_arg, __func__, "commit");

    repo = GET_SLOT(commit, Rf_install("repo"));
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git2r_commit_lookup(&commit_obj, repository, commit);
    if (GIT_OK != err)
        goto cleanup;

    err = git_commit_tree(&tree, commit_obj);
    if (GIT_OK != err)
        goto cleanup;

    PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_tree")));
    git2r_tree_init((git_tree*)tree, repo, result);

cleanup:
    if (commit_obj)
        git_commit_free(commit_obj);

    if (tree)
        git_tree_free(tree);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != result)
        UNPROTECT(1);

    if (GIT_OK != err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return result;
}

/**
 * Init slots in S4 class git_commit
 *
 * @param source a commit object
 * @param repo S4 class git_repository that contains the blob
 * @param dest S4 class git_commit to initialize
 * @return void
 */
void git2r_commit_init(git_commit *source, SEXP repo, SEXP dest)
{
    const char *message;
    const char *summary;
    const git_signature *author;
    const git_signature *committer;
    char sha[GIT_OID_HEXSZ + 1];

    git_oid_fmt(sha, git_commit_id(source));
    sha[GIT_OID_HEXSZ] = '\0';
    SET_SLOT(dest,
             Rf_install("sha"),
             ScalarString(mkChar(sha)));

    author = git_commit_author(source);
    if (author) {
        SEXP sexp_author;

        PROTECT(sexp_author = NEW_OBJECT(MAKE_CLASS("git_signature")));
        git2r_signature_init(author, sexp_author);
        SET_SLOT(dest, Rf_install("author"), sexp_author);
        UNPROTECT(1);
    }

    committer = git_commit_committer(source);
    if (committer) {
        SEXP sexp_committer;

        PROTECT(sexp_committer = NEW_OBJECT(MAKE_CLASS("git_signature")));
        git2r_signature_init(committer, sexp_committer);
        SET_SLOT(dest, Rf_install("committer"), sexp_committer);
        UNPROTECT(1);
    }

    summary = git_commit_summary(source);
    if (summary) {
        SET_SLOT(dest,
                 Rf_install("summary"),
                 ScalarString(mkChar(summary)));
    }

    message = git_commit_message(source);
    if (message) {
        SET_SLOT(dest,
                 Rf_install("message"),
                 ScalarString(mkChar(message)));
    }

    SET_SLOT(dest, Rf_install("repo"), duplicate(repo));
}

/**
 * Parents of a commit
 *
 * @param commit S4 class git_commit
 * @return list of S4 class git_commit objects
 */
SEXP git2r_commit_parent_list(SEXP commit)
{
    int err;
    size_t i, n;
    SEXP repo;
    SEXP list = R_NilValue;
    git_commit *commit_obj = NULL;
    git_repository *repository = NULL;

    if (GIT_OK != git2r_arg_check_commit(commit))
        git2r_error(git2r_err_commit_arg, __func__, "commit");

    repo = GET_SLOT(commit, Rf_install("repo"));
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(git2r_err_invalid_repository, __func__, NULL);

    err = git2r_commit_lookup(&commit_obj, repository, commit);
    if (GIT_OK != err)
        goto cleanup;

    n = git_commit_parentcount(commit_obj);
    PROTECT(list = allocVector(VECSXP, n));

    for (i = 0; i < n; i++) {
        git_commit *parent = NULL;
        SEXP tmp;

        err = git_commit_parent(&parent, commit_obj, i);
        if (GIT_OK != err)
            goto cleanup;

        PROTECT(tmp = NEW_OBJECT(MAKE_CLASS("git_commit")));
        git2r_commit_init(parent, repo, tmp);
        SET_VECTOR_ELT(list, i, tmp);
        UNPROTECT(1);
        git_commit_free(parent);
    }

cleanup:
    if (commit_obj)
        git_commit_free(commit_obj);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != list)
        UNPROTECT(1);

    if (GIT_OK != err)
        git2r_error(git2r_err_from_libgit2, __func__, giterr_last()->message);

    return list;
}
