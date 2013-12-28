/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013  Stefan Widgren
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  git2r is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Rdefines.h>
#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>

#include <git2.h>
#include <git2/repository.h>

/**
 * Free repository.
 *
 * @param repo
 */
static void repo_finalizer(SEXP repo)
{
    if (NULL == R_ExternalPtrAddr(repo))
        return;
    git_repository_free((git_repository *)R_ExternalPtrAddr(repo));
    R_ClearExternalPtr(repo);
}

/**
 * Get repo slot from S4 class repository
 *
 * @param repo
 * @return
 */
static git_repository* get_repository(const SEXP repo)
{
    SEXP class_name;
    SEXP slot;
    git_repository *r;

    if (R_NilValue == repo || S4SXP != TYPEOF(repo))
        error("Invalid repository");

    class_name = getAttrib(repo, R_ClassSymbol);
    if (0 != strcmp(CHAR(STRING_ELT(class_name, 0)), "repository"))
        error("Invalid repository");

    slot = GET_SLOT(repo, Rf_install("repo"));
    if (R_NilValue == slot)
        error("Invalid repository");

    r = (git_repository *)R_ExternalPtrAddr(slot);
    if (NULL == r)
        error("Invalid repository");

    return r;
}

/**
 * Check if repository is bare.
 *
 * @param repo S4 class to an open repository
 * @return
 */
SEXP is_bare(const SEXP repo)
{
    if (git_repository_is_bare(get_repository(repo)))
        return ScalarLogical(TRUE);
    return ScalarLogical(FALSE);
}

/**
 * Check if repository is empty.
 *
 * @param repo S4 class to an open repository
 * @return
 */
SEXP is_empty(const SEXP repo)
{
    if (git_repository_is_empty(get_repository(repo)))
        return ScalarLogical(TRUE);
    return ScalarLogical(FALSE);
}

/**
 * Repository.
 *
 * @param path
 * @return S4 class repository
 */
SEXP repository(const SEXP path)
{
    SEXP repo = R_NilValue;
    SEXP xp = R_NilValue;
    git_repository *r = NULL;
    int err;

    if (R_NilValue == path)
        error("'path' equals R_NilValue.");
    if (!isString(path))
        error("'path' must be a string.");

    PROTECT(repo = NEW_OBJECT(MAKE_CLASS("repository")));
    if (R_NilValue == repo)
        error("Unable to make S4 class repository");

    err = git_repository_open(&r, CHAR(STRING_ELT(path, 0)));
    if (err) {
        const git_error *e = giterr_last();
        error("Error %d/%d: %s\n", error, e->klass, e->message);
    }

    PROTECT(xp = R_MakeExternalPtr(r, R_NilValue, R_NilValue));
    R_RegisterCFinalizerEx(xp, repo_finalizer, TRUE);
    SET_SLOT(repo, Rf_install("repo"), xp);

    UNPROTECT(2);

    return repo;
}

static const R_CallMethodDef callMethods[] =
{
    {"is_bare", (DL_FUNC)&is_bare, 1},
    {"is_empty", (DL_FUNC)&is_empty, 1},
    {"repository", (DL_FUNC)&repository, 1},
    {NULL, NULL, 0}
};

/**
 * Register routines to R.
 *
 * @param info Information about the DLL being loaded
 */
void
R_init_gitr(DllInfo *info)
{
    R_registerRoutines(info, NULL, callMethods, NULL, NULL);
}
