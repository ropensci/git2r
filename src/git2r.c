/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2015 The git2r contributors
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

#include "git2r_blame.h"
#include "git2r_blob.h"
#include "git2r_branch.h"
#include "git2r_checkout.h"
#include "git2r_clone.h"
#include "git2r_config.h"
#include "git2r_commit.h"
#include "git2r_diff.h"
#include "git2r_error.h"
#include "git2r_graph.h"
#include "git2r_index.h"
#include "git2r_libgit2.h"
#include "git2r_merge.h"
#include "git2r_note.h"
#include "git2r_object.h"
#include "git2r_odb.h"
#include "git2r_push.h"
#include "git2r_reference.h"
#include "git2r_reflog.h"
#include "git2r_remote.h"
#include "git2r_repository.h"
#include "git2r_reset.h"
#include "git2r_revparse.h"
#include "git2r_revwalk.h"
#include "git2r_signature.h"
#include "git2r_stash.h"
#include "git2r_status.h"
#include "git2r_tag.h"

static const R_CallMethodDef callMethods[] =
{
    {"git2r_blame_file", (DL_FUNC)&git2r_blame_file, 2},
    {"git2r_blob_content", (DL_FUNC)&git2r_blob_content, 1},
    {"git2r_blob_create_fromdisk", (DL_FUNC)&git2r_blob_create_fromdisk, 2},
    {"git2r_blob_create_fromworkdir", (DL_FUNC)&git2r_blob_create_fromworkdir, 2},
    {"git2r_blob_is_binary", (DL_FUNC)&git2r_blob_is_binary, 1},
    {"git2r_blob_rawsize", (DL_FUNC)&git2r_blob_rawsize, 1},
    {"git2r_branch_canonical_name", (DL_FUNC)&git2r_branch_canonical_name, 1},
    {"git2r_branch_create", (DL_FUNC)&git2r_branch_create, 3},
    {"git2r_branch_delete", (DL_FUNC)&git2r_branch_delete, 1},
    {"git2r_branch_get_upstream", (DL_FUNC)&git2r_branch_get_upstream, 1},
    {"git2r_branch_is_head", (DL_FUNC)&git2r_branch_is_head, 1},
    {"git2r_branch_list", (DL_FUNC)&git2r_branch_list, 2},
    {"git2r_branch_remote_name", (DL_FUNC)&git2r_branch_remote_name, 1},
    {"git2r_branch_remote_url", (DL_FUNC)&git2r_branch_remote_url, 1},
    {"git2r_branch_rename", (DL_FUNC)&git2r_branch_rename, 3},
    {"git2r_branch_set_upstream", (DL_FUNC)&git2r_branch_set_upstream, 2},
    {"git2r_branch_target", (DL_FUNC)&git2r_branch_target, 1},
    {"git2r_branch_upstream_canonical_name", (DL_FUNC)&git2r_branch_upstream_canonical_name, 1},
    {"git2r_checkout_tree", (DL_FUNC)&git2r_checkout_tree, 3},
    {"git2r_clone", (DL_FUNC)&git2r_clone, 6},
    {"git2r_commit", (DL_FUNC)&git2r_commit, 4},
    {"git2r_commit_parent_list", (DL_FUNC)&git2r_commit_parent_list, 1},
    {"git2r_commit_tree", (DL_FUNC)&git2r_commit_tree, 1},
    {"git2r_config_get", (DL_FUNC)&git2r_config_get, 1},
    {"git2r_config_set", (DL_FUNC)&git2r_config_set, 2},
    {"git2r_diff", (DL_FUNC)&git2r_diff, 5},
    {"git2r_graph_ahead_behind", (DL_FUNC)&git2r_graph_ahead_behind, 2},
    {"git2r_graph_descendant_of", (DL_FUNC)&git2r_graph_descendant_of, 2},
    {"git2r_index_add_all", (DL_FUNC)&git2r_index_add_all, 3},
    {"git2r_index_remove_bypath", (DL_FUNC)&git2r_index_remove_bypath, 2},
    {"git2r_libgit2_features", (DL_FUNC)&git2r_libgit2_features, 0},
    {"git2r_libgit2_version", (DL_FUNC)&git2r_libgit2_version, 0},
    {"git2r_merge_base", (DL_FUNC)&git2r_merge_base, 2},
    {"git2r_merge_branch", (DL_FUNC)&git2r_merge_branch, 3},
    {"git2r_merge_fetch_heads", (DL_FUNC)&git2r_merge_fetch_heads, 2},
    {"git2r_note_create", (DL_FUNC)&git2r_note_create, 7},
    {"git2r_note_default_ref", (DL_FUNC)&git2r_note_default_ref, 1},
    {"git2r_notes", (DL_FUNC)&git2r_notes, 2},
    {"git2r_note_remove", (DL_FUNC)&git2r_note_remove, 3},
    {"git2r_object_lookup", (DL_FUNC)&git2r_object_lookup, 2},
    {"git2r_odb_blobs", (DL_FUNC)&git2r_odb_blobs, 1},
    {"git2r_odb_hash", (DL_FUNC)&git2r_odb_hash, 1},
    {"git2r_odb_hashfile", (DL_FUNC)&git2r_odb_hashfile, 1},
    {"git2r_odb_objects", (DL_FUNC)&git2r_odb_objects, 1},
    {"git2r_push", (DL_FUNC)&git2r_push, 4},
    {"git2r_reference_list", (DL_FUNC)&git2r_reference_list, 1},
    {"git2r_reflog_list", (DL_FUNC)&git2r_reflog_list, 2},
    {"git2r_remote_add", (DL_FUNC)&git2r_remote_add, 3},
    {"git2r_remote_fetch", (DL_FUNC)&git2r_remote_fetch, 4},
    {"git2r_remote_list", (DL_FUNC)&git2r_remote_list, 1},
    {"git2r_remote_remove", (DL_FUNC)&git2r_remote_remove, 2},
    {"git2r_remote_rename", (DL_FUNC)&git2r_remote_rename, 3},
    {"git2r_remote_url", (DL_FUNC)&git2r_remote_url, 2},
    {"git2r_repository_can_open", (DL_FUNC)&git2r_repository_can_open, 1},
    {"git2r_repository_discover", (DL_FUNC)&git2r_repository_discover, 1},
    {"git2r_repository_fetch_heads", (DL_FUNC)&git2r_repository_fetch_heads, 1},
    {"git2r_repository_head", (DL_FUNC)&git2r_repository_head, 1},
    {"git2r_repository_head_detached", (DL_FUNC)&git2r_repository_head_detached, 1},
    {"git2r_repository_init", (DL_FUNC)&git2r_repository_init, 2},
    {"git2r_repository_is_bare", (DL_FUNC)&git2r_repository_is_bare, 1},
    {"git2r_repository_is_empty", (DL_FUNC)&git2r_repository_is_empty, 1},
    {"git2r_repository_is_shallow", (DL_FUNC)&git2r_repository_is_shallow, 1},
    {"git2r_repository_set_head", (DL_FUNC)&git2r_repository_set_head, 2},
    {"git2r_repository_set_head_detached", (DL_FUNC)&git2r_repository_set_head_detached, 1},
    {"git2r_repository_workdir", (DL_FUNC)&git2r_repository_workdir, 1},
    {"git2r_reset", (DL_FUNC)&git2r_reset, 2},
    {"git2r_revparse_single", (DL_FUNC)&git2r_revparse_single, 2},
    {"git2r_revwalk_contributions", (DL_FUNC)&git2r_revwalk_contributions, 4},
    {"git2r_revwalk_list", (DL_FUNC)&git2r_revwalk_list, 5},
    {"git2r_signature_default", (DL_FUNC)&git2r_signature_default, 1},
    {"git2r_stash_drop", (DL_FUNC)&git2r_stash_drop, 2},
    {"git2r_stash_list", (DL_FUNC)&git2r_stash_list, 1},
    {"git2r_stash_save", (DL_FUNC)&git2r_stash_save, 6},
    {"git2r_status_list", (DL_FUNC)&git2r_status_list, 5},
    {"git2r_tag_create", (DL_FUNC)&git2r_tag_create, 4},
    {"git2r_tag_list", (DL_FUNC)&git2r_tag_list, 1},
    {NULL, NULL, 0}
};

/**
 * Load 'git2r'
 *  - Register routines to R.
 *  - Initialize libgit2
 *
 * @param info Information about the DLL being loaded
 */
void
R_init_git2r(DllInfo *info)
{
    R_registerRoutines(info, NULL, callMethods, NULL, NULL);
    git_libgit2_init();
}

/**
 * Unload 'git2r'
 *
 * @param info Information about the DLL being unloaded
 */
void
R_unload_git2r(DllInfo *info)
{
    git_libgit2_shutdown();
}
