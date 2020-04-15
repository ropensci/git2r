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

#ifndef INCLUDE_git2r_deprecated_h
#define INCLUDE_git2r_deprecated_h

/* The constants GIT_OBJ_ANY, GIT_OBJ_BLOB, GIT_OBJ_COMMIT,
 * GIT_OBJ_TAG_GIT_OBJ_TREE and GIT_REF_OID are deprecated in
 * libgit2. Use GIT_OBJECT_ANY, GIT_OBJECT_BLOB, GIT_OBJECT_COMMIT,
 * GIT_OBJECT_TAG_GIT_OBJECT_TREE and GIT_REFERENCE_DIRECT, if
 * available, instead. */
#if defined(GIT2R_HAVE_OBJECT_ANY)
# define GIT2R_OBJECT_ANY GIT_OBJECT_ANY
# define GIT2R_OBJECT_BLOB GIT_OBJECT_BLOB
# define GIT2R_OBJECT_COMMIT GIT_OBJECT_COMMIT
# define GIT2R_OBJECT_TAG GIT_OBJECT_TAG
# define GIT2R_OBJECT_TREE GIT_OBJECT_TREE
# define GIT2R_REFERENCE_DIRECT GIT_REFERENCE_DIRECT
# define GIT2R_REFERENCE_SYMBOLIC GIT_REFERENCE_SYMBOLIC
#else
# define GIT2R_OBJECT_ANY GIT_OBJ_ANY
# define GIT2R_OBJECT_BLOB GIT_OBJ_BLOB
# define GIT2R_OBJECT_COMMIT GIT_OBJ_COMMIT
# define GIT2R_OBJECT_TAG GIT_OBJ_TAG
# define GIT2R_OBJECT_TREE GIT_OBJ_TREE
# define GIT2R_REFERENCE_DIRECT GIT_REF_OID
# define GIT2R_REFERENCE_SYMBOLIC GIT_REF_SYMBOLIC
#endif

/* The function 'git_buf_free' is deprecated in libgit2. Use
 * 'git_buf_dispose', if available, instead. */
#if defined(GIT2R_HAVE_BUF_DISPOSE)
# define GIT2R_BUF_DISPOSE git_buf_dispose
#else
# define GIT2R_BUF_DISPOSE git_buf_free
#endif

#if defined(GIT2R_HAVE_GIT_ERROR)
# define GIT2R_ERROR_SET_STR git_error_set_str
# define GIT2R_ERROR_LAST git_error_last
# define GIT2R_ERROR_SET_OOM git_error_set_oom
# define GIT2R_ERROR_NONE GIT_ERROR_NONE
# define GIT2R_ERROR_OS GIT_ERROR_OS
# define GIT2R_ERROR_NOMEMORY GIT_ERROR_NOMEMORY
# define GIT2R_ERROR_CONFIG GIT_ERROR_CONFIG
# define GIT2R_OBJECT_T git_object_t
#else
# define GIT2R_ERROR_SET_STR giterr_set_str
# define GIT2R_ERROR_LAST giterr_last
# define GIT2R_ERROR_SET_OOM giterr_set_oom
# define GIT2R_ERROR_NONE GITERR_NONE
# define GIT2R_ERROR_OS GITERR_OS
# define GIT2R_ERROR_NOMEMORY GITERR_NOMEMORY
# define GIT2R_ERROR_CONFIG GITERR_CONFIG
# define GIT2R_OBJECT_T git_otype
#endif

#if defined(GIT2R_LIBGIT2_V0_99_0_RENAMES)
# define GIT2R_CREDENTIAL git_credential
# define GIT2R_CREDENTIAL_SSH_KEY GIT_CREDENTIAL_SSH_KEY
# define GIT2R_CREDENTIAL_SSH_KEY_NEW git_credential_ssh_key_new
# define GIT2R_CREDENTIAL_USERPASS_PLAINTEXT_NEW git_credential_userpass_plaintext_new
# define GIT2R_CREDENTIAL_SSH_KEY_FROM_AGENT git_credential_ssh_key_from_agent
# define GIT2R_CREDENTIAL_USERPASS_PLAINTEXT GIT_CREDENTIAL_USERPASS_PLAINTEXT
# define GIT2R_INDEXER_PROGRESS git_indexer_progress
# define GIT2R_OID_IS_ZERO git_oid_is_zero
# define GIT2R_BLOB_CREATE_FROM_DISK git_blob_create_from_disk
# define GIT2R_BLOB_CREATE_FROM_WORKDIR git_blob_create_from_workdir
#else
# define GIT2R_CREDENTIAL git_cred
# define GIT2R_CREDENTIAL_SSH_KEY GIT_CREDTYPE_SSH_KEY
# define GIT2R_CREDENTIAL_SSH_KEY_NEW git_cred_ssh_key_new
# define GIT2R_CREDENTIAL_USERPASS_PLAINTEXT_NEW git_cred_userpass_plaintext_new
# define GIT2R_CREDENTIAL_SSH_KEY_FROM_AGENT git_cred_ssh_key_from_agent
# define GIT2R_CREDENTIAL_USERPASS_PLAINTEXT GIT_CREDTYPE_USERPASS_PLAINTEXT
# define GIT2R_INDEXER_PROGRESS git_transfer_progress
# define GIT2R_OID_IS_ZERO git_oid_iszero
# define GIT2R_BLOB_CREATE_FROM_DISK git_blob_create_fromdisk
# define GIT2R_BLOB_CREATE_FROM_WORKDIR git_blob_create_fromworkdir
#endif

#endif
