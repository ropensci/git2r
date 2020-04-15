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

#ifndef INCLUDE_git2r_transfer_h
#define INCLUDE_git2r_transfer_h

#include <R.h>
#include <Rinternals.h>
#include <git2.h>
#include "git2r_deprecated.h"

/**
 * Data structure to hold information when performing a clone, fetch
 * or push operation.
 */
typedef struct {
    int received_progress;
    int received_done;
    int verbose;

    /* Used in the 'git2r_cred_acquire_cb' callback to determine if to
     * use the 'ssh-agent' to find the ssh key for authentication.
     * Only used when credentials equals R_NilValue. */
    int use_ssh_agent;

    /* Used in the 'git2r_cred_acquire_cb' callback to determine if to
     * to search for the 'id_rsa' ssh key for authentication. Only
     * used when credentials equals R_NilValue. FIXME: This is
     * currently always set to zero, i.e. git2r does not automatically
     * search for the 'id_rsa' ssh key. */
    int use_ssh_key;

    SEXP credentials;
} git2r_transfer_data;

#ifdef WIN32
#  define GIT2R_TRANSFER_DATA_INIT {0, 0, 0, 1, 0, R_NilValue}
#else
#  define GIT2R_TRANSFER_DATA_INIT {0, 0, 0, 1, 0, R_NilValue}
#endif

void git2r_transfer_progress_init(
    const GIT2R_INDEXER_PROGRESS *source,
    SEXP dest);

#endif
