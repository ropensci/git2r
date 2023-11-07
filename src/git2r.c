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

/** @file git2r.c
 *  @brief R bindings to the libgit2 library
 *
 *  These functions are called from R with .Call to interface the
 *  libgit2 library from R.
 *
 */

#include "git2r_arg.h"
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
#include "git2r_tree.h"
#include <R_ext/Rdynload.h>

#define CALLDEF(name, n) {#name, (DL_FUNC) &name, n}

static const R_CallMethodDef callMethods[] =
{
    CALLDEF(git2r_blame_file, 2),
    CALLDEF(git2r_blob_content, 2),
    CALLDEF(git2r_blob_create_fromdisk, 2),
    CALLDEF(git2r_blob_create_fromworkdir, 2),
    CALLDEF(git2r_blob_is_binary, 1),
    CALLDEF(git2r_blob_rawsize, 1),
    CALLDEF(git2r_branch_canonical_name, 1),
    CALLDEF(git2r_branch_create, 3),
    CALLDEF(git2r_branch_delete, 1),
    CALLDEF(git2r_branch_get_upstream, 1),
    CALLDEF(git2r_branch_is_head, 1),
    CALLDEF(git2r_branch_list, 2),
    CALLDEF(git2r_branch_remote_name, 1),
    CALLDEF(git2r_branch_remote_url, 1),
    CALLDEF(git2r_branch_rename, 3),
    CALLDEF(git2r_branch_set_upstream, 2),
    CALLDEF(git2r_branch_target, 1),
    CALLDEF(git2r_branch_upstream_canonical_name, 1),
    CALLDEF(git2r_checkout_path, 2),
    CALLDEF(git2r_checkout_tree, 3),
    CALLDEF(git2r_clone, 7),
    CALLDEF(git2r_commit, 4),
    CALLDEF(git2r_commit_parent_list, 1),
    CALLDEF(git2r_commit_tree, 1),
    CALLDEF(git2r_config_find_file, 1),
    CALLDEF(git2r_config_get, 1),
    CALLDEF(git2r_config_get_logical, 2),
    CALLDEF(git2r_config_get_string, 2),
    CALLDEF(git2r_config_set, 2),
    CALLDEF(git2r_diff, 12),
    CALLDEF(git2r_graph_ahead_behind, 2),
    CALLDEF(git2r_graph_descendant_of, 2),
    CALLDEF(git2r_index_add_all, 3),
    CALLDEF(git2r_index_remove_bypath, 2),
    CALLDEF(git2r_libgit2_features, 0),
    CALLDEF(git2r_libgit2_version, 0),
    CALLDEF(git2r_merge_base, 2),
    CALLDEF(git2r_merge_branch, 4),
    CALLDEF(git2r_merge_fetch_heads, 2),
    CALLDEF(git2r_note_create, 7),
    CALLDEF(git2r_note_default_ref, 1),
    CALLDEF(git2r_notes, 2),
    CALLDEF(git2r_note_remove, 3),
    CALLDEF(git2r_object_lookup, 2),
    CALLDEF(git2r_odb_blobs, 1),
    CALLDEF(git2r_odb_hash, 1),
    CALLDEF(git2r_odb_hashfile, 1),
    CALLDEF(git2r_odb_objects, 1),
    CALLDEF(git2r_push, 4),
    CALLDEF(git2r_reference_dwim, 2),
    CALLDEF(git2r_reference_list, 1),
    CALLDEF(git2r_reflog_list, 2),
    CALLDEF(git2r_remote_add, 3),
    CALLDEF(git2r_remote_fetch, 6),
    CALLDEF(git2r_remote_list, 1),
    CALLDEF(git2r_remote_remove, 2),
    CALLDEF(git2r_remote_rename, 3),
    CALLDEF(git2r_remote_set_url, 3),
    CALLDEF(git2r_remote_url, 2),
    CALLDEF(git2r_remote_ls, 3),
    CALLDEF(git2r_repository_can_open, 1),
    CALLDEF(git2r_repository_discover, 2),
    CALLDEF(git2r_repository_fetch_heads, 1),
    CALLDEF(git2r_repository_head, 1),
    CALLDEF(git2r_repository_head_detached, 1),
    CALLDEF(git2r_repository_init, 3),
    CALLDEF(git2r_repository_is_bare, 1),
    CALLDEF(git2r_repository_is_empty, 1),
    CALLDEF(git2r_repository_is_shallow, 1),
    CALLDEF(git2r_repository_set_head, 2),
    CALLDEF(git2r_repository_set_head_detached, 1),
    CALLDEF(git2r_repository_workdir, 1),
    CALLDEF(git2r_reset, 2),
    CALLDEF(git2r_reset_default, 2),
    CALLDEF(git2r_revparse_single, 2),
    CALLDEF(git2r_revwalk_contributions, 4),
    CALLDEF(git2r_revwalk_list, 6),
    CALLDEF(git2r_revwalk_list2, 7),
    CALLDEF(git2r_signature_default, 1),
    CALLDEF(git2r_ssl_cert_locations, 2),
    CALLDEF(git2r_stash_apply, 2),
    CALLDEF(git2r_stash_drop, 2),
    CALLDEF(git2r_stash_list, 1),
    CALLDEF(git2r_stash_pop, 2),
    CALLDEF(git2r_stash_save, 6),
    CALLDEF(git2r_status_list, 6),
    CALLDEF(git2r_tag_create, 5),
    CALLDEF(git2r_tag_delete, 2),
    CALLDEF(git2r_tag_list, 1),
    CALLDEF(git2r_tree_walk, 2),
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
    R_useDynamicSymbols(info, FALSE);
    R_forceSymbols(info, TRUE);
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
    GIT2R_UNUSED(info);
    git_libgit2_shutdown();
}
