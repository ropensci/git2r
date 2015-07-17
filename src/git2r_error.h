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

#ifndef INCLUDE_git2r_error_h
#define INCLUDE_git2r_error_h

#include "git2.h"

/**
 * Error messages
 */
extern const char git2r_err_alloc_memory_buffer[];
extern const char git2r_err_branch_not_local[];
extern const char git2r_err_branch_not_remote[];
extern const char git2r_err_checkout_tree[];
extern const char git2r_err_invalid_refname[];
extern const char git2r_err_invalid_remote[];
extern const char git2r_err_invalid_repository[];
extern const char git2r_err_nothing_added_to_commit[];
extern const char git2r_err_object_type[];
extern const char git2r_err_reference[];
extern const char git2r_err_repo_init[];
extern const char git2r_err_revparse_single[];
extern const char git2r_err_unexpected_config_level[];
extern const char git2r_err_unable_to_authenticate[];

/**
 * Error messages specific to argument checking
 */
extern const char git2r_err_blob_arg[];
extern const char git2r_err_branch_arg[];
extern const char git2r_err_commit_arg[];
extern const char git2r_err_credentials_arg[];
extern const char git2r_err_diff_arg[];
extern const char git2r_err_fetch_heads_arg[];
extern const char git2r_err_filename_arg[];
extern const char git2r_err_sha_arg[];
extern const char git2r_err_integer_arg[];
extern const char git2r_err_integer_gte_zero_arg[];
extern const char git2r_err_list_arg[];
extern const char git2r_err_logical_arg[];
extern const char git2r_err_note_arg[];
extern const char git2r_err_signature_arg[];
extern const char git2r_err_string_arg[];
extern const char git2r_err_string_vec_arg[];
extern const char git2r_err_tag_arg[];
extern const char git2r_err_tree_arg[];

void git2r_error(
    const char *func_name,
    const git_error *err,
    const char *msg1,
    const char *msg2);

#endif
