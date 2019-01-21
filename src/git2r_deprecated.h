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

#endif
