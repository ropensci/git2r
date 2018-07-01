/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2018 The git2r contributors
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

#ifndef INCLUDE_git2r_stash_h
#define INCLUDE_git2r_stash_h

#include <R.h>
#include <Rinternals.h>

SEXP git2r_stash_apply(SEXP repo, SEXP index);
SEXP git2r_stash_drop(SEXP repo, SEXP index);
SEXP git2r_stash_pop(SEXP repo, SEXP index);
SEXP git2r_stash_save(SEXP repo, SEXP message, SEXP index, SEXP untracked,
                      SEXP ignored, SEXP stasher);
SEXP git2r_stash_list(SEXP repo);

#endif
