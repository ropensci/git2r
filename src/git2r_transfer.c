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

#include <Rdefines.h>
#include "git2r_transfer.h"

/**
 * Init slots in S4 class git_transfer_progress
 *
 * @param source A git_transfer_progress object
 * @param dest S4 class git_transfer_progress to initialize
 * @return void
 */
void git2r_transfer_progress_init(
    const git_transfer_progress *source,
    SEXP dest)
{
    SET_SLOT(dest,
             Rf_install("total_objects"),
             ScalarInteger(source->total_objects));

    SET_SLOT(dest,
             Rf_install("indexed_objects"),
             ScalarInteger(source->indexed_objects));

    SET_SLOT(dest,
             Rf_install("received_objects"),
             ScalarInteger(source->received_objects));

    SET_SLOT(dest,
             Rf_install("local_objects"),
             ScalarInteger(source->local_objects));

    SET_SLOT(dest,
             Rf_install("total_deltas"),
             ScalarInteger(source->total_deltas));

    SET_SLOT(dest,
             Rf_install("indexed_deltas"),
             ScalarInteger(source->indexed_deltas));

    SET_SLOT(dest,
             Rf_install("received_bytes"),
             ScalarInteger(source->received_bytes));
}
