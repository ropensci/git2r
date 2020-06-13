/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2020 The git2r contributors
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

#include <R_ext/Visibility.h>
#include "git2r_deprecated.h"
#include "git2r_S3.h"
#include "git2r_transfer.h"

/**
 * Init slots in S3 class git_transfer_progress
 *
 * @param source A GIT2R_INDEXER_PROGRESS object
 * @param dest S3 class git_transfer_progress to initialize
 * @return void
 */
void attribute_hidden
git2r_transfer_progress_init(
    const GIT2R_INDEXER_PROGRESS *source,
    SEXP dest)
{
    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_transfer_progress__total_objects,
        Rf_ScalarInteger(source->total_objects));
    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_transfer_progress__indexed_objects,
        Rf_ScalarInteger(source->indexed_objects));
    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_transfer_progress__received_objects,
        Rf_ScalarInteger(source->received_objects));
    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_transfer_progress__local_objects,
        Rf_ScalarInteger(source->local_objects));
    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_transfer_progress__total_deltas,
        Rf_ScalarInteger(source->total_deltas));
    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_transfer_progress__indexed_deltas,
        Rf_ScalarInteger(source->indexed_deltas));
    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_transfer_progress__received_bytes,
        Rf_ScalarInteger(source->received_bytes));
}
