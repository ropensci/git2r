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

#ifndef INCLUDE_git2r_revwalk_h
#define INCLUDE_git2r_revwalk_h

#include <R.h>
#include <Rinternals.h>

SEXP git2r_revwalk_contributions(SEXP repo, SEXP topological, SEXP time, SEXP reverse);
SEXP git2r_revwalk_list(SEXP repo, SEXP sha, SEXP topological, SEXP time, SEXP reverse, SEXP max_n);
SEXP git2r_revwalk_list2(SEXP repo, SEXP sha, SEXP topological, SEXP time, SEXP reverse, SEXP max_n, SEXP path);

#endif
