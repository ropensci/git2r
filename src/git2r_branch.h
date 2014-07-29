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

#ifndef INCLUDE_git2r_branch_h
#define INCLUDE_git2r_branch_h

#include <R.h>
#include <Rinternals.h>
#include "git2.h"

SEXP git2r_branch_canonical_name(SEXP branch);
SEXP git2r_branch_create(
    SEXP branch_name,
    SEXP commit,
    SEXP force,
    SEXP signature,
    SEXP message);
SEXP git2r_branch_delete(SEXP branch);
int git2r_branch_init(
    const git_reference *source,
    git_branch_t type,
    SEXP repo,
    SEXP dest);
SEXP git2r_branch_is_head(SEXP branch);
SEXP git2r_branch_list(SEXP repo, SEXP flags);
SEXP git2r_branch_remote_name(SEXP branch);
SEXP git2r_branch_remote_url(SEXP branch);
SEXP git2r_branch_rename(
    SEXP branch,
    SEXP new_branch_name,
    SEXP force,
    SEXP signature,
    SEXP message);
SEXP git2r_branch_target(SEXP branch);
SEXP git2r_branch_get_upstream(SEXP branch);
SEXP git2r_branch_set_upstream(SEXP branch, SEXP upstream_name);
SEXP git2r_branch_upstream_canonical_name(SEXP branch);

#endif
