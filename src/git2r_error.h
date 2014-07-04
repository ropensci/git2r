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

#ifndef INCLUDE_git2r_error_h
#define INCLUDE_git2r_error_h

#include <R.h>
#include <Rinternals.h>

/**
 * Error messages
 */

extern const char git2r_err_alloc_memory_buffer[];
extern const char git2r_err_invalid_repository[];
extern const char git2r_err_nothing_added_to_commit[];
extern const char git2r_err_unexpected_config_level[];
extern const char git2r_err_unexpected_head_of_branch[];
extern const char git2r_err_unexpected_type_of_branch[];

int git2r_arg_check_blob(SEXP arg);
int git2r_arg_check_branch(SEXP arg);
int git2r_arg_check_commit(SEXP arg);
int git2r_arg_check_hex(SEXP arg);
int git2r_arg_check_integer(SEXP arg);
int git2r_arg_check_logical(SEXP arg);
int git2r_arg_check_note(SEXP arg);
int git2r_arg_check_signature(SEXP arg);
int git2r_arg_check_string(SEXP arg);
int git2r_arg_check_tag(SEXP arg);
int git2r_arg_check_tree(SEXP arg);

#endif
