/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2024 The git2r contributors
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
