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

#ifndef INCLUDE_git2r_commit_h
#define INCLUDE_git2r_commit_h

#include <R.h>
#include <Rinternals.h>

#include "git2.h"

SEXP git2r_commit(
    SEXP repo,
    SEXP message,
    SEXP author,
    SEXP committer);
int git2r_commit_lookup(
    git_commit **out,
    git_repository *repository,
    SEXP commit);
SEXP git2r_commit_tree(SEXP commit);
void git2r_commit_init(git_commit *source, SEXP repo, SEXP dest);
SEXP git2r_commit_parent_list(SEXP commit);

#endif
