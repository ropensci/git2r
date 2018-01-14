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

#ifndef INCLUDE_git2r_objects_h
#define INCLUDE_git2r_objects_h

#include <R.h>
#include <Rinternals.h>

extern const char *git2r_S3_class__git_merge_result;
extern const char *git2r_S3_items__git_merge_result[];
enum {
    git2r_S3_item__git_merge_result__up_to_date,
    git2r_S3_item__git_merge_result__fast_forward,
    git2r_S3_item__git_merge_result__conflicts,
    git2r_S3_item__git_merge_result__sha};


extern const char *git2r_S3_class__git_transfer_progress;
extern const char *git2r_S3_items__git_transfer_progress[];
enum {
    git2r_S3_item__git_transfer_progress__total_objects,
    git2r_S3_item__git_transfer_progress__indexed_objects,
    git2r_S3_item__git_transfer_progress__received_objects,
    git2r_S3_item__git_transfer_progress__local_objects,
    git2r_S3_item__git_transfer_progress__total_deltas,
    git2r_S3_item__git_transfer_progress__indexed_deltas,
    git2r_S3_item__git_transfer_progress__received_bytes};

SEXP getListElement(SEXP list, const char *str);

#endif
