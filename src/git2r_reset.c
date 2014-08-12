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
#include "git2.h"

#include "git2r_arg.h"
#include "git2r_commit.h"
#include "git2r_error.h"
#include "git2r_repository.h"
#include "git2r_reset.h"
#include "git2r_signature.h"

/**
 * Reset current HEAD to the specified state
 *
 * @param commit The commit to which the HEAD should be moved to.
 * @param reset_type Kind of reset operation to perform. 'soft' means
 * the Head will be moved to the commit. 'mixed' reset will trigger a
 * 'soft' reset, plus the index will be replaced with the content of
 * the commit tree. 'hard' reset will trigger a 'mixed' reset and the
 * working directory will be replaced with the content of the index.
 * @param msg The one line long message to the reflog. The default
 * value is "reset: moving".
 * @param who The identity that will be used to populate the
 * reflog entry. Default is the default signature.
 * @return R_NilValue
 */
SEXP git2r_reset(SEXP commit, SEXP reset_type, SEXP msg, SEXP who)
{
    int err;
    SEXP repo;
    git_signature *signature = NULL;
    git_commit *target = NULL;
    git_repository *repository = NULL;

    if (0 != git2r_arg_check_commit(commit))
        Rf_error(git2r_err_commit_arg, "commit");
    if (0 != git2r_arg_check_integer(reset_type))
        Rf_error(git2r_err_integer_arg, "reset_type");
    if (0 != git2r_arg_check_string(msg))
        Rf_error(git2r_err_string_arg, "msg");
    if (0 != git2r_arg_check_signature(who))
        Rf_error(git2r_err_signature_arg, "who");

    err = git2r_signature_from_arg(&signature, who);
    if (err < 0)
        goto cleanup;

    repo = GET_SLOT(commit, Rf_install("repo"));
    repository = git2r_repository_open(repo);
    if (!repository)
        Rf_error(git2r_err_invalid_repository);

    err = git2r_commit_lookup(&target, repository, commit);
    if (err < 0)
        goto cleanup;

    err = git_reset(repository,
                    target,
                    INTEGER(reset_type)[0],
                    signature,
                    CHAR(STRING_ELT(msg, 0)));

cleanup:
    if (signature)
        git_signature_free(signature);

    if (target)
        git_commit_free(target);

    if (repository)
        git_repository_free(repository);

    if (err < 0)
        Rf_error("Error: %s\n", giterr_last()->message);

    return R_NilValue;
}
