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
# define GIT2R_INDEXER_PROGRESS git_indexer_progress
# define GIT2R_BLOB_CREATE_FROM_DISK git_blob_create_from_disk
# define GIT2R_BLOB_CREATE_FROM_WORKDIR git_blob_create_from_workdir
#else
# define GIT2R_INDEXER_PROGRESS git_transfer_progress
# define GIT2R_BLOB_CREATE_FROM_DISK git_blob_create_fromdisk
# define GIT2R_BLOB_CREATE_FROM_WORKDIR git_blob_create_fromworkdir
#endif

#endif
