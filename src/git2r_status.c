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

#include "git2r_error.h"
#include "git2r_repository.h"
#include "git2r_status.h"

/**
 * Count number of ignored files
 *
 * @param status
 * @return The number of files
 */
static size_t count_ignored_files(git_status_list *status_list)
{
    size_t i = 0;
    size_t ignored = 0;
    size_t count = git_status_list_entrycount(status_list);

    for (; i < count; ++i) {
        const git_status_entry *s = git_status_byindex(status_list, i);

        if (s->status == GIT_STATUS_IGNORED)
            ignored++;
    }

    return ignored;
}

/**
 * Count number of changes in index
 *
 * @param status
 * @return The number of changes
 */
static size_t count_staged_changes(git_status_list *status_list)
{
    size_t i = 0;
    size_t changes = 0;
    size_t count = git_status_list_entrycount(status_list);

    for (; i < count; ++i) {
        const git_status_entry *s = git_status_byindex(status_list, i);

        if (s->status == GIT_STATUS_CURRENT)
            continue;

        if (s->status & GIT_STATUS_INDEX_NEW)
            changes++;
        else if (s->status & GIT_STATUS_INDEX_MODIFIED)
            changes++;
        else if (s->status & GIT_STATUS_INDEX_DELETED)
            changes++;
        else if (s->status & GIT_STATUS_INDEX_RENAMED)
            changes++;
        else if (s->status & GIT_STATUS_INDEX_TYPECHANGE)
            changes++;
    }

    return changes;
}

/**
 * Count number of changes in workdir relative to index
 *
 * @param status
 * @return The number of changes
 */
static size_t count_unstaged_changes(git_status_list *status_list)
{
    size_t i = 0;
    size_t changes = 0;
    size_t count = git_status_list_entrycount(status_list);

    for (; i < count; ++i) {
        const git_status_entry *s = git_status_byindex(status_list, i);

        if (s->status == GIT_STATUS_CURRENT || s->index_to_workdir == NULL)
            continue;

        if (s->status & GIT_STATUS_WT_MODIFIED)
            changes++;
        else if (s->status & GIT_STATUS_WT_DELETED)
            changes++;
        else if (s->status & GIT_STATUS_WT_RENAMED)
            changes++;
        else if (s->status & GIT_STATUS_WT_TYPECHANGE)
            changes++;
    }

    return changes;
}

/**
 * Count number of untracked files
 *
 * @param status
 * @return The number of files
 */
static size_t count_untracked_files(git_status_list *status_list)
{
    size_t i = 0;
    size_t untracked = 0;
    size_t count = git_status_list_entrycount(status_list);

    for (; i < count; ++i) {
        const git_status_entry *s = git_status_byindex(status_list, i);

        if (s->status == GIT_STATUS_WT_NEW)
            untracked++;
    }

    return untracked;
}

/**
 * Add ignored files
 *
 * @param list
 * @param list_index
 * @param status_list
 * @return void
 */
static void list_ignored_files(SEXP list,
                               size_t list_index,
                               git_status_list *status_list)
{
    size_t i = 0, j = 0, count;
    SEXP sub_list, sub_list_names, item;

    /* Create list with the correct number of entries */
    count = count_ignored_files(status_list);
    PROTECT(sub_list = allocVector(VECSXP, count));
    PROTECT(sub_list_names = allocVector(STRSXP, count));

    /* i index the entrycount. j index the added change in list */
    count = git_status_list_entrycount(status_list);
    for (; i < count; i++) {
        const git_status_entry *s = git_status_byindex(status_list, i);

        if (s->status == GIT_STATUS_IGNORED) {
            SET_STRING_ELT(sub_list_names, j, mkChar("ignored"));
            PROTECT(item = allocVector(STRSXP, 1));
            SET_STRING_ELT(item, 0, mkChar(s->index_to_workdir->old_file.path));
            SET_VECTOR_ELT(sub_list, j, item);
            UNPROTECT(1);
            j++;
        }
    }

    setAttrib(sub_list, R_NamesSymbol, sub_list_names);
    SET_VECTOR_ELT(list, list_index, sub_list);
    UNPROTECT(2);
}

/**
 * Add changes in index
 *
 * @param list
 * @param list_index
 * @param status_list
 * @return void
 */
static void list_staged_changes(SEXP list,
                                size_t list_index,
                                git_status_list *status_list)
{
    size_t i = 0, j = 0, count;
    SEXP sub_list, sub_list_names, item;

    /* Create list with the correct number of entries */
    count = count_staged_changes(status_list);
    PROTECT(sub_list = allocVector(VECSXP, count));
    PROTECT(sub_list_names = allocVector(STRSXP, count));

    /* i index the entrycount. j index the added change in list */
    count = git_status_list_entrycount(status_list);
    for (; i < count; i++) {
        char *istatus = NULL;
        const char *old_path, *new_path;
        const git_status_entry *s = git_status_byindex(status_list, i);

        if (s->status == GIT_STATUS_CURRENT)
            continue;

        if (s->status & GIT_STATUS_INDEX_NEW)
            istatus = "new";
        else if (s->status & GIT_STATUS_INDEX_MODIFIED)
            istatus = "modified";
        else if (s->status & GIT_STATUS_INDEX_DELETED)
            istatus = "deleted";
        else if (s->status & GIT_STATUS_INDEX_RENAMED)
            istatus = "renamed";
        else if (s->status & GIT_STATUS_INDEX_TYPECHANGE)
            istatus = "typechange";

        if (!istatus)
            continue;
        SET_STRING_ELT(sub_list_names, j, mkChar(istatus));

        old_path = s->head_to_index->old_file.path;
        new_path = s->head_to_index->new_file.path;

        if (old_path && new_path && strcmp(old_path, new_path)) {
            PROTECT(item = allocVector(STRSXP, 2));
            SET_STRING_ELT(item, 0, mkChar(old_path));
            SET_STRING_ELT(item, 1, mkChar(new_path));
        } else {
            PROTECT(item = allocVector(STRSXP, 1));
            SET_STRING_ELT(item, 0, mkChar(old_path ? old_path : new_path));
        }

        SET_VECTOR_ELT(sub_list, j, item);
        UNPROTECT(1);
        j++;
    }

    setAttrib(sub_list, R_NamesSymbol, sub_list_names);
    SET_VECTOR_ELT(list, list_index, sub_list);
    UNPROTECT(2);
}

/**
 * Add changes in workdir relative to index
 *
 * @param list
 * @param list_index
 * @param status_list
 * @return void
 */
static void list_unstaged_changes(SEXP list,
                                  size_t list_index,
                                  git_status_list *status_list)
{
    size_t i = 0, j = 0, count;
    SEXP sub_list, sub_list_names, item;

    /* Create list with the correct number of entries */
    count = count_unstaged_changes(status_list);
    PROTECT(sub_list = allocVector(VECSXP, count));
    PROTECT(sub_list_names = allocVector(STRSXP, count));

    /* i index the entrycount. j index the added change in list */
    count = git_status_list_entrycount(status_list);
    for (; i < count; i++) {
        char *wstatus = NULL;
        const char *old_path, *new_path;
        const git_status_entry *s = git_status_byindex(status_list, i);

        if (s->status == GIT_STATUS_CURRENT || s->index_to_workdir == NULL)
            continue;

        if (s->status & GIT_STATUS_WT_MODIFIED)
            wstatus = "modified";
        else if (s->status & GIT_STATUS_WT_DELETED)
            wstatus = "deleted";
        else if (s->status & GIT_STATUS_WT_RENAMED)
            wstatus = "renamed";
        else if (s->status & GIT_STATUS_WT_TYPECHANGE)
            wstatus = "typechange";

        if (!wstatus)
            continue;
        SET_STRING_ELT(sub_list_names, j, mkChar(wstatus));

        old_path = s->index_to_workdir->old_file.path;
        new_path = s->index_to_workdir->new_file.path;

        if (old_path && new_path && strcmp(old_path, new_path)) {
            PROTECT(item = allocVector(STRSXP, 2));
            SET_STRING_ELT(item, 0, mkChar(old_path));
            SET_STRING_ELT(item, 1, mkChar(new_path));
        } else {
            PROTECT(item = allocVector(STRSXP, 1));
            SET_STRING_ELT(item, 0, mkChar(old_path ? old_path : new_path));
        }

        SET_VECTOR_ELT(sub_list, j, item);
        UNPROTECT(1);
        j++;
    }

    setAttrib(sub_list, R_NamesSymbol, sub_list_names);
    SET_VECTOR_ELT(list, list_index, sub_list);
    UNPROTECT(2);
}

/**
 * Add untracked files
 *
 * @param list
 * @param list_index
 * @param status_list
 * @return void
 */
static void list_untracked_files(SEXP list,
                                 size_t list_index,
                                 git_status_list *status_list)
{
    size_t i = 0, j = 0, count;
    SEXP sub_list, sub_list_names, item;

    /* Create list with the correct number of entries */
    count = count_untracked_files(status_list);
    PROTECT(sub_list = allocVector(VECSXP, count));
    PROTECT(sub_list_names = allocVector(STRSXP, count));

    /* i index the entrycount. j index the added change in list */
    count = git_status_list_entrycount(status_list);
    for (; i < count; i++) {
        const git_status_entry *s = git_status_byindex(status_list, i);

        if (s->status == GIT_STATUS_WT_NEW) {
            SET_STRING_ELT(sub_list_names, j, mkChar("untracked"));
            PROTECT(item = allocVector(STRSXP, 1));
            SET_STRING_ELT(item, 0, mkChar(s->index_to_workdir->old_file.path));
            SET_VECTOR_ELT(sub_list, j, item);
            UNPROTECT(1);
            j++;
        }
    }

    setAttrib(sub_list, R_NamesSymbol, sub_list_names);
    SET_VECTOR_ELT(list, list_index, sub_list);
    UNPROTECT(2);
}

/**
 * Get state of the repository working directory and the staging area.
 *
 * @param repo S4 class git_repository
 * @return VECXSP with status
 */
SEXP status(SEXP repo, SEXP staged, SEXP unstaged, SEXP untracked, SEXP ignored)
{
    int err;
    size_t i=0, count;
    SEXP list, list_names;
    git_repository *repository;
    git_status_list *status_list = NULL;
    git_status_options opts = GIT_STATUS_OPTIONS_INIT;

    if (check_logical_arg(staged)
        || check_logical_arg(unstaged)
        || check_logical_arg(untracked)
        || check_logical_arg(ignored))
        error("Invalid arguments to status");

    repository = get_repository(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    opts.show  = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
    opts.flags = GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX |
        GIT_STATUS_OPT_SORT_CASE_SENSITIVELY;

    if (LOGICAL(untracked)[0])
        opts.flags |= GIT_STATUS_OPT_INCLUDE_UNTRACKED;
    if (LOGICAL(ignored)[0])
        opts.flags |= GIT_STATUS_OPT_INCLUDE_IGNORED;
    err = git_status_list_new(&status_list, repository, &opts);
    if (err < 0)
        goto cleanup;

    count = LOGICAL(staged)[0] +
        LOGICAL(unstaged)[0] +
        LOGICAL(untracked)[0] +
        LOGICAL(ignored)[0];

    PROTECT(list = allocVector(VECSXP, count));
    PROTECT(list_names = allocVector(STRSXP, count));

    if (LOGICAL(staged)[0]) {
        SET_STRING_ELT(list_names, i, mkChar("staged"));
        list_staged_changes(list, i, status_list);
        i++;
    }

    if (LOGICAL(unstaged)[0]) {
        SET_STRING_ELT(list_names, i, mkChar("unstaged"));
        list_unstaged_changes(list, i, status_list);
        i++;
    }

    if (LOGICAL(untracked)[0]) {
        SET_STRING_ELT(list_names, i, mkChar("untracked"));
        list_untracked_files(list, i, status_list);
        i++;
    }

    if (LOGICAL(ignored)[0]) {
        SET_STRING_ELT(list_names, i, mkChar("ignored"));
        list_ignored_files(list, i, status_list);
    }

    setAttrib(list, R_NamesSymbol, list_names);

cleanup:
    if (status_list)
        git_status_list_free(status_list);

    if (repository)
        git_repository_free(repository);

    UNPROTECT(2);

    if (err < 0) {
        const git_error *e = giterr_last();
        error("Error %d/%d: %s\n", err, e->klass, e->message);
    }

    return list;
}
