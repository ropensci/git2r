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

/**
 * Error messages
 */

const char git2r_err_alloc_memory_buffer[] = "Unable to allocate memory buffer";
const char git2r_err_invalid_repository[] = "Invalid repository";
const char git2r_err_nothing_added_to_commit[] = "Nothing added to commit";
const char git2r_err_unexpected_config_level[] = "Unexpected config level";
const char git2r_err_unexpected_type_of_branch[] = "Unexpected type of branch";
const char git2r_err_unexpected_head_of_branch[] = "Unexpected head of branch";

/**
 * Error messages specific to argument checking
 */
const char git2r_err_blob_arg[] =
    "Error: '%s' must be a S4 class git_blob";
const char git2r_err_branch_arg[] =
    "Error: '%s' must be a S4 class git_branch";
const char git2r_err_commit_arg[] =
    "Error: '%s' must be a S4 class git_commit";
const char git2r_err_credentials_arg[] =
    "Error: '%s' must be a S4 class with credentials";
const char git2r_err_integer_arg[] =
    "Error: '%s' must be an integer vector of length one with non NA value";
const char git2r_err_logical_arg[] =
    "Error: '%s' must be logical vector of length one with non NA value";
const char git2r_err_signature_arg[] =
    "Error: '%s' must be a S4 class git_signature";
const char git2r_err_string_arg[] =
    "Error: '%s' must be a character vector of length one with non NA value";
const char git2r_err_string_vec_arg[] =
    "Error: '%s' must be a character vector";
const char git2r_err_tag_arg[] =
    "Error: '%s' must be a S4 class git_tag";
const char git2r_err_tree_arg[] =
    "Error: '%s' must be a S4 class git_tree";
