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

#include <Rdefines.h>
#include "git2r_tree.h"

/**
 * Init slots in S4 class git_tree
 *
 * @param source a tree
 * @param repo S4 class git_repository that contains the tree
 * @param dest S4 class git_tree to initialize
 * @return void
 */
void init_tree(const git_tree *source, SEXP repo, SEXP dest)
{
    SEXP filemode, id, type, name;
    size_t i, n;
    const git_oid *oid;
    char hex[GIT_OID_HEXSZ + 1];
    const git_tree_entry *entry;

    oid = git_tree_id(source);
    git_oid_tostr(hex, sizeof(hex), oid);
    SET_SLOT(dest,
             Rf_install("hex"),
             ScalarString(mkChar(hex)));

    n = git_tree_entrycount(source);
    PROTECT(filemode = allocVector(INTSXP, n));
    PROTECT(id = allocVector(STRSXP, n));
    PROTECT(type = allocVector(STRSXP, n));
    PROTECT(name = allocVector(STRSXP, n));

    SET_SLOT(dest, Rf_install("filemode"), filemode);
    SET_SLOT(dest, Rf_install("id"), id);
    SET_SLOT(dest, Rf_install("type"), type);
    SET_SLOT(dest, Rf_install("name"), name);
    UNPROTECT(4);

    for (i = 0; i < n; ++i) {
        entry = git_tree_entry_byindex(source, i);
        git_oid_tostr(hex, sizeof(hex), git_tree_entry_id(entry));
        INTEGER(filemode)[i] = git_tree_entry_filemode(entry);
        SET_STRING_ELT(id, i, mkChar(hex));
        SET_STRING_ELT(type, i, mkChar(git_object_type2string(git_tree_entry_type(entry))));
        SET_STRING_ELT(name, i, mkChar(git_tree_entry_name(entry)));
    }

    SET_SLOT(dest, Rf_install("repo"), duplicate(repo));
}
