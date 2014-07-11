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

#include "git2r_cred.h"

/**
 * Callback if the remote host requires authentication in order to connect to it
 *
 * @param out :TODO:DOCUMENTATION:
 * @param url :TODO:DOCUMENTATION:
 * @param username_from_url :TODO:DOCUMENTATION:
 * @param allowed_types :TODO:DOCUMENTATION:
 * @param payload :TODO:DOCUMENTATION:
 * @return :TODO:DOCUMENTATION:
 */
int git2r_cred_acquire_cb(
    git_cred **out,
    const char *url,
    const char *username_from_url,
    unsigned int allowed_types,
    void *payload)
{
    giterr_set_str(
        GITERR_SSH,
        "The git2r credential callback isn't implemented. Sorry");
    return -1;
}
