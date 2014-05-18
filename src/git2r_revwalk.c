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
#include "git2r_error.h"

/**
 * Count number of revisions.
 *
 * @param walker
 * @return
 */
static size_t git2r_revwalk_count(git_revwalk *walker)
{
    size_t n = 0;
    git_oid oid;

    while (!git_revwalk_next(&oid, walker))
        n++;
    return n;
}

/**
 * List revisions
 *
 * @param repo S4 class git_repository
 * @return
 */
SEXP git2r_revwalk_list(SEXP repo)
{
    int i=0;
    int err = 0;
    SEXP list;
    size_t n = 0;
    git_revwalk *walker = NULL;
    git_repository *repository = NULL;

    repository = git2r_repository_open(repo);
    if (!repository)
        error(git2r_err_invalid_repository);

    if (git_repository_is_empty(repository)) {
        /* No commits, create empty list */
        PROTECT(list = allocVector(VECSXP, 0));
        goto cleanup;
    }

    err = git_revwalk_new(&walker, repository);
    if (err < 0)
        goto cleanup;

    err = git_revwalk_push_head(walker);
    if (err < 0)
        goto cleanup;

    /* Count number of revisions before creating the list */
    n = git2r_revwalk_count(walker);

    /* Create list to store result */
    PROTECT(list = allocVector(VECSXP, n));

    git_revwalk_reset(walker);
    err = git_revwalk_push_head(walker);
    if (err < 0)
        goto cleanup;

    for (;;) {
        git_commit *commit;
        SEXP sexp_commit;
        git_oid oid;

        err = git_revwalk_next(&oid, walker);
        if (err < 0) {
            if (GIT_ITEROVER == err)
                err = 0;
            goto cleanup;
        }

        err = git_commit_lookup(&commit, repository, &oid);
        if (err < 0)
            goto cleanup;

        PROTECT(sexp_commit = NEW_OBJECT(MAKE_CLASS("git_commit")));
        git2r_commit_init(commit, repo, sexp_commit);
        SET_VECTOR_ELT(list, i, sexp_commit);
        UNPROTECT(1);
        i++;

        git_commit_free(commit);
    }

cleanup:
    if (walker)
        git_revwalk_free(walker);

    if (repository)
        git_repository_free(repository);

    UNPROTECT(1);

    if (err < 0)
        error("Error: %s\n", giterr_last()->message);

    return list;
}
