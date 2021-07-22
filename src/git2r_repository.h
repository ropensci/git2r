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

#ifndef INCLUDE_git2r_repository_h
#define INCLUDE_git2r_repository_h

#include <R.h>
#include <Rinternals.h>

#include <git2.h>

git_repository* git2r_repository_open(SEXP repo);
SEXP git2r_repository_can_open(SEXP path);
SEXP git2r_repository_discover(SEXP path, SEXP ceiling);
SEXP git2r_repository_fetch_heads(SEXP repo);
SEXP git2r_repository_head(SEXP repo);
SEXP git2r_repository_head_detached(SEXP repo);
SEXP git2r_repository_init(SEXP path, SEXP bare, SEXP branch);
SEXP git2r_repository_is_bare(SEXP repo);
SEXP git2r_repository_is_empty(SEXP repo);
SEXP git2r_repository_is_shallow(SEXP repo);
SEXP git2r_repository_set_head(SEXP repo, SEXP ref_name);
SEXP git2r_repository_set_head_detached(SEXP commit);
SEXP git2r_repository_workdir(SEXP repo);

#endif
