/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2023 The git2r contributors
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

#ifndef INCLUDE_git2r_blob_h
#define INCLUDE_git2r_blob_h

#include <R.h>
#include <Rinternals.h>
#include <git2.h>

SEXP git2r_blob_content(SEXP blob, SEXP raw);
SEXP git2r_blob_create_fromdisk(SEXP repo, SEXP path);
SEXP git2r_blob_create_fromworkdir(SEXP repo, SEXP relative_path);
void git2r_blob_init(const git_blob *source, SEXP repo, SEXP dest);
SEXP git2r_blob_is_binary(SEXP blob);
SEXP git2r_blob_rawsize(SEXP blob);

#endif
