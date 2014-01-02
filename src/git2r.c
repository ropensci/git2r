/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2014  Stefan Widgren
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

static void init_reference(git_reference *ref, SEXP reference);
static git_repository* get_repository(const SEXP repo);
static size_t number_of_branches(git_repository *repo, int flags);

/**
 * Error messages
 */

const char err_alloc_VECSXP[] = "Unable to allocate VECSXP";
const char err_alloc_STRSXP[] = "Unable to allocate STRSXP";
const char err_alloc_S4_git_branch[] = "Unable to allocate S4 git_branch";
const char err_alloc_char_buffer[] = "Unable to allocate character buffer";
const char err_unexpected_type_of_branch[] = "Unexpected type of branch";
const char err_unexpected_head_of_branch[] = "Unexpected head of branch";

/**
 * List branches in a repository
 *
 * @param repo S4 class to an open repository
 * @return VECXSP with S4 objects of class git_branch
 */
SEXP branches(const SEXP repo, const SEXP flags)
{
    SEXP list = R_NilValue;
    int err = 0;
    const char* err_msg = NULL;
    git_branch_iterator *iter = NULL;
    size_t i = 0, n = 0;
    size_t protected = 0;

    /* Count number of branches before creating the list */
    n = number_of_branches(get_repository(repo), INTEGER(flags)[0]);

    PROTECT(list = allocVector(VECSXP, n));
    if (R_NilValue == list) {
        err = 1;
        err_msg = err_alloc_VECSXP;
        goto cleanup;
    }
    protected++;

    err = git_branch_iterator_new(&iter,
                                  get_repository(repo),
                                  INTEGER(flags)[0]);
    if (err)
        goto cleanup;

    for (;;) {
        SEXP branch;
        git_branch_t type;
        git_reference *ref;
        const char *refname;

        err = git_branch_next(&ref, &type, iter);
        if (GIT_ITEROVER == err) {
            err = 0;
            break;
        } else if (err) {
            err = 1;
            goto cleanup;
        }

        PROTECT(branch = NEW_OBJECT(MAKE_CLASS("git_branch")));
        if (R_NilValue == branch) {
            err = 1;
            err_msg = err_alloc_S4_git_branch;
            goto cleanup;
        }
        protected++;

        refname = git_reference_name(ref);
        init_reference(ref, branch);

        switch (type) {
        case GIT_BRANCH_LOCAL:
            break;
        case GIT_BRANCH_REMOTE: {
            char *buf;
            size_t buf_size;
            git_remote *remote = NULL;

            buf_size = git_branch_remote_name(NULL,
                                              0,
                                              get_repository(repo),
                                              refname);
            buf = malloc(buf_size * sizeof(char));
            if (NULL == buf) {
                err = 1;
                err_msg = err_alloc_char_buffer;
                goto cleanup;
            }

            git_branch_remote_name(buf,
                                   buf_size,
                                   get_repository(repo),
                                   refname);

            SET_SLOT(branch, Rf_install("remote"), ScalarString(mkChar(buf)));

            err = git_remote_load(&remote, get_repository(repo), buf);
            if (err < 0) {
                err = git_remote_create_inmemory(&remote,
                                                 get_repository(repo),
                                                 NULL,
                                                 buf);
                if (err < 0) {
                    err = 1;
                    goto cleanup;
                }
            }

            SET_SLOT(branch,
                     Rf_install("url"),
                     ScalarString(mkChar(git_remote_url(remote))));

            free(buf);
            git_remote_free(remote);
            break;
        }
        default:
            err = 1;
            err_msg = err_unexpected_type_of_branch;
            goto cleanup;
        }

        switch (git_branch_is_head(ref)) {
        case 0:
            SET_SLOT(branch, Rf_install("head"), ScalarLogical(0));
            break;
        case 1:
            SET_SLOT(branch, Rf_install("head"), ScalarLogical(1));
            break;
        default:
            err = 1;
            err_msg = err_unexpected_head_of_branch;
            goto cleanup;
        }

        git_reference_free(ref);
        SET_VECTOR_ELT(list, i, branch);
        if (protected) {
            UNPROTECT(1);
            protected--;
        }
        i++;
    }

cleanup:
    git_branch_iterator_free(iter);

    if (protected)
        UNPROTECT(protected);

    if (err) {
        if (err_msg) {
            error(err_msg);
        } else {
            const git_error *e = giterr_last();
            error("Error %d: %s\n", e->klass, e->message);
        }
    }

    return list;
}

/**
 * Free repository.
 *
 * @param repo EXTPTRSXP to a git_repository
 */
static void repo_finalizer(SEXP repo)
{
    if (EXTPTRSXP != TYPEOF(repo))
        error("'repo' not an EXTPTRSXP");
    if (NULL == R_ExternalPtrAddr(repo))
        return;
    git_repository_free((git_repository *)R_ExternalPtrAddr(repo));
    R_ClearExternalPtr(repo);
}

/**
 * Free walker.
 *
 * @param walker EXTPTRSXP to a git_revwalk
 */
static void walker_finalizer(SEXP walker)
{
    if (EXTPTRSXP != TYPEOF(walker))
        error("'walker' not an EXTPTRSXP");
    if (NULL == R_ExternalPtrAddr(walker))
        return;
    git_revwalk_free((git_revwalk *)R_ExternalPtrAddr(walker));
    R_ClearExternalPtr(walker);
}

/**
 * Get repo slot from S4 class git_repository
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
    if (0 != strcmp(CHAR(STRING_ELT(class_name, 0)), "git_repository"))
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
 * Get walker slot from S4 class git_repository
 *
 * @param repo
 * @return
 */
static git_revwalk* get_walker(const SEXP repo)
{
    SEXP class_name;
    SEXP slot;
    git_revwalk *walker;

    if (R_NilValue == repo || S4SXP != TYPEOF(repo))
        error("Invalid repository");

    class_name = getAttrib(repo, R_ClassSymbol);
    if (0 != strcmp(CHAR(STRING_ELT(class_name, 0)), "git_repository"))
        error("Invalid repository");

    slot = GET_SLOT(repo, Rf_install("walker"));
    if (R_NilValue == slot)
        error("Invalid repository");

    walker = (git_revwalk *)R_ExternalPtrAddr(slot);
    if (NULL == walker)
        error("Invalid repository");

    return walker;
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

static void init_reference(git_reference *ref, SEXP reference)
{
    char out[41];
    out[40] = '\0';

    SET_SLOT(reference,
             Rf_install("name"),
             ScalarString(mkChar(git_reference_name(ref))));

    SET_SLOT(reference,
             Rf_install("shorthand"),
             ScalarString(mkChar(git_reference_shorthand(ref))));

    switch (git_reference_type(ref)) {
    case GIT_REF_OID:
        SET_SLOT(reference, Rf_install("type"), ScalarInteger(GIT_REF_OID));
        git_oid_fmt(out, git_reference_target(ref));
        SET_SLOT(reference, Rf_install("hex"), ScalarString(mkChar(out)));
        break;
    case GIT_REF_SYMBOLIC:
        SET_SLOT(reference, Rf_install("type"), ScalarInteger(GIT_REF_SYMBOLIC));
        SET_SLOT(reference,
                 Rf_install("target"),
                 ScalarString(mkChar(git_reference_symbolic_target(ref))));
        break;
    default:
        error("Unexpected reference type");
    }
}

static void init_signature(git_signature *sig, SEXP signature)
{
    SEXP when;

    SET_SLOT(signature,
             Rf_install("name"),
             ScalarString(mkChar(sig->name)));

    SET_SLOT(signature,
             Rf_install("email"),
             ScalarString(mkChar(sig->email)));

    when = GET_SLOT(signature, Rf_install("when"));

    SET_SLOT(when,
             Rf_install("time"),
             ScalarReal((double)sig->when.time));

    SET_SLOT(when,
             Rf_install("offset"),
             ScalarReal((double)sig->when.offset));
}

/**
 * Count number of branches.
 *
 * @param repo
 * @param flags
 * @return
 */
static size_t number_of_branches(git_repository *repo, int flags)
{
    size_t n = 0;
    int err;
    git_branch_iterator *iter;
    git_branch_t type;
    git_reference *ref;

    err = git_branch_iterator_new(&iter, repo, flags);
    if (err) {
        const git_error *e = giterr_last();
        error("Error %d/%d: %s\n", error, e->klass, e->message);
    }

    for (;;) {
        err = git_branch_next(&ref, &type, iter);
        if (err)
            break;
        git_reference_free(ref);
        n++;
    }

    if (GIT_ITEROVER != err) {
        const git_error *e = giterr_last();
        error("Error %d/%d: %s\n", error, e->klass, e->message);
    }

    git_branch_iterator_free(iter);

    return n;
}

/**
 * Count number of revisions.
 *
 * @param walker
 * @return
 */
static size_t number_of_revisions(git_revwalk *walker)
{
    size_t n = 0;
    git_oid oid;

    while (!git_revwalk_next(&oid, walker)) {
        n++;
    }

    return n;
}

/**
 * Get all references that can be found in a repository.
 *
 * @param repo S4 class to an open repository
 * @return VECXSP with S4 objects of class git_reference
 */
SEXP references(const SEXP repo)
{
    int i, err;
    git_strarray l;
    SEXP list;
    SEXP names;
    SEXP reference;
    git_reference *ref;
    const char *refname;
    char out[41];
    out[40] = '\0';

    err = git_reference_list(&l, get_repository(repo));

    /* :TODO:FIX: Check error code */

    PROTECT(list = allocVector(VECSXP, l.count));
    if (R_NilValue == list) {
        error("Unable to list references");
    }

    PROTECT(names = allocVector(STRSXP, l.count));
    if (R_NilValue == names) {
        UNPROTECT(1);
        error("Unable to list references");
    }

    for (i = 0; i < l.count; i++) {
        PROTECT(reference = NEW_OBJECT(MAKE_CLASS("git_reference")));
        if (R_NilValue == reference) {
            UNPROTECT(2);
            error("Unable to list references");
        }

        refname = l.strings[i];
        git_reference_lookup(&ref, get_repository(repo), refname);
        init_reference(ref, reference);

        SET_STRING_ELT(names, i, mkChar(refname));
        SET_VECTOR_ELT(list, i, reference);
        UNPROTECT(1);
    }

    git_strarray_free(&l);

    setAttrib(list, R_NamesSymbol, names);
    UNPROTECT(2);

    return list;
}

/**
 * Get the configured remotes for a repo
 *
 * @param repo
 * @return
 */
SEXP remotes(const SEXP repo)
{
    int i, err;
    git_strarray l;
    SEXP r;

    err = git_remote_list(&l, get_repository(repo));

    /* :TODO:FIX: Check error code */

    PROTECT(r = allocVector(STRSXP, l.count));
    for (i = 0; i < l.count; i++)
        SET_STRING_ELT(r, i, mkChar(l.strings[i]));
    UNPROTECT(1);

    git_strarray_free(&l);

    return r;
}

/**
 * Get the remote's url
 *
 * @param repo
 * @return
 */
SEXP remote_url(const SEXP repo, const SEXP remote)
{
    SEXP url = R_NilValue;
    size_t len = LENGTH(remote);
    size_t i = 0;
    int err;
    git_remote *r = NULL;

    PROTECT(url = allocVector(STRSXP, len));

    for (; i < len; i++) {
        err = git_remote_load(&r, get_repository(repo), CHAR(STRING_ELT(remote, i)));
        /* :TODO:FIX: Check error code */

        SET_STRING_ELT(url, i, mkChar(git_remote_url(r)));
        git_remote_free(r);
    }

    UNPROTECT(1);

    return url;
}

/**
 * Repository.
 *
 * @param path
 * @return S4 class repository
 */
SEXP repository(const SEXP path)
{
    SEXP sexp_repo = R_NilValue;
    SEXP xp_repo = R_NilValue;
    SEXP xp_walker = R_NilValue;
    git_repository *repo = NULL;
    git_revwalk *walker;
    int err;

    if (R_NilValue == path)
        error("'path' equals R_NilValue.");
    if (!isString(path))
        error("'path' must be a string.");

    PROTECT(sexp_repo = NEW_OBJECT(MAKE_CLASS("git_repository")));
    if (R_NilValue == sexp_repo)
        error("Unable to make S4 class git_repository");

    /* Initialize external pointer to repository */
    err = git_repository_open(&repo, CHAR(STRING_ELT(path, 0)));
    if (err) {
        UNPROTECT(1);
        const git_error *e = giterr_last();
        error("Error %d/%d: %s\n", error, e->klass, e->message);
    }
    PROTECT(xp_repo = R_MakeExternalPtr(repo, R_NilValue, R_NilValue));
    R_RegisterCFinalizerEx(xp_repo, repo_finalizer, TRUE);
    SET_SLOT(sexp_repo, Rf_install("repo"), xp_repo);

    /* Initialize external pointer to revision walker */
    err = git_revwalk_new(&walker, repo);
    if (err) {
        UNPROTECT(2);
        const git_error *e = giterr_last();
        error("Error %d/%d: %s\n", error, e->klass, e->message);
    }
    PROTECT(xp_walker = R_MakeExternalPtr(walker, R_NilValue, R_NilValue));
    R_RegisterCFinalizerEx(xp_walker, walker_finalizer, TRUE);
    SET_SLOT(sexp_repo, Rf_install("walker"), xp_walker);

    UNPROTECT(3);

    return sexp_repo;
}

SEXP revisions(const SEXP repository)
{
    int i=0;
    int err;
    SEXP sexp_author;
    SEXP sexp_commit;
    git_signature *sig;
    SEXP list;
    size_t n;
    git_commit *commit;
    git_revwalk *walker;
    git_oid oid;
    git_repository *repo;
    const char *message;
    char oid_hex[GIT_OID_HEXSZ + 1];

    repo = get_repository(repository);
    walker = get_walker(repository);
    git_revwalk_push_head(walker);

    /* Count number of revisions before creating the list */
    n = number_of_revisions(walker);

    /* Create list to store result */
    PROTECT(list = allocVector(VECSXP, n));
    if (R_NilValue == list)
        error("Unable to list revisions");

    git_revwalk_reset(walker);
    git_revwalk_push_head(walker);
    while (!git_revwalk_next(&oid, walker)) {
        git_oid_tostr(oid_hex, sizeof(oid_hex), &oid);

        /* :TODO:FIX: Check err */
        err = git_commit_lookup(&commit, repo, &oid);

        PROTECT(sexp_commit = NEW_OBJECT(MAKE_CLASS("git_commit")));
        if (R_NilValue == sexp_commit)
            error("Unable to make S4 class git_commit");

        sig = git_commit_author(commit);
        if (sig) {
            PROTECT(sexp_author = NEW_OBJECT(MAKE_CLASS("git_signature")));
            if (R_NilValue == sexp_author)
                error("Unable to make S4 class git_signature");
            init_signature(sig, sexp_author);
            SET_SLOT(sexp_commit, Rf_install("author"), sexp_author);
            UNPROTECT(1);
        }

        message  = git_commit_message(commit);
        if (message) {
            SET_SLOT(sexp_commit,
                     Rf_install("message"),
                     ScalarString(mkChar(message)));
        }

        SET_VECTOR_ELT(list, i, sexp_commit);
        UNPROTECT(1);
        i++;

        git_commit_free(commit);
    }

    UNPROTECT(1);

    return list;
}

/**
 * Get state of repository.
 *
 * @param repo
 * @return
 */
SEXP state(const SEXP repo)
{
    return ScalarInteger(git_repository_state(get_repository(repo)));
}

/**
 * Get all tags that can be found in a repository.
 *
 * @param repo S4 class to an open repository
 * @return VECXSP with S4 objects of class tag
 */
SEXP tags(const SEXP repo)
{
    int i, err;
    git_strarray l;
    SEXP list;
    SEXP names;
    SEXP tag;
    git_reference *ref;
    const char *tagname;
    char* buf;
    size_t tagname_len;
    git_tag *t;
    const git_oid *oid;
    const git_signature *signature;

    err = git_tag_list(&l, get_repository(repo));

    /* :TODO:FIX: Check error code */

    PROTECT(list = allocVector(VECSXP, l.count));
    if (R_NilValue == list) {
        error("Unable to list tags");
    }

    PROTECT(names = allocVector(STRSXP, l.count));
    if (R_NilValue == names) {
        UNPROTECT(1);
        error("Unable to list tags");
    }

    for (i = 0; i < l.count; i++) {
        PROTECT(tag = NEW_OBJECT(MAKE_CLASS("git_tag")));
        if (R_NilValue == tag) {
            UNPROTECT(2);
            error("Unable to list tags");
        }

        tagname = l.strings[i];

        /* Prefix tagname with "refs/tags/" to lookup reference */
        tagname_len = strlen(tagname);
        buf = malloc((tagname_len+11)*sizeof(char));
        if (!buf) {
            error("Unable to list tags");
            UNPROTECT(3);
            error("Unable to list tags");
        }
        *buf = '\0';
        strncat(buf, "refs/tags/", 10);
        strncat(buf, tagname, tagname_len);
        git_reference_lookup(&ref, get_repository(repo), buf);
        free(buf);
        init_reference(ref, tag);

        /* Fill in signature for tag */
        /* oid = git_reference_target(ref); */
        /* if (oid) { */
        /*     err = git_tag_lookup(&t, get_repository(repo), oid); */
        /*     signature = git_tag_tagger(t); */
        /*     SET_SLOT(GET_SLOT(tag, Rf_install("sig")), */
        /*              Rf_install("name"), */
        /*              ScalarString(mkChar(signature->name))); */
        /* } */

        SET_STRING_ELT(names, i, mkChar(tagname));
        SET_VECTOR_ELT(list, i, tag);
        UNPROTECT(1);
    }

    git_strarray_free(&l);

    setAttrib(list, R_NamesSymbol, names);
    UNPROTECT(2);

    return list;
}

/**
 * Get workdir of repository.
 *
 * @param repo
 * @return
 */
SEXP workdir(const SEXP repo)
{
    return ScalarString(mkChar(git_repository_workdir(get_repository(repo))));
}

static const R_CallMethodDef callMethods[] =
{
    {"branches", (DL_FUNC)&branches, 2},
    {"is_bare", (DL_FUNC)&is_bare, 1},
    {"is_empty", (DL_FUNC)&is_empty, 1},
    {"references", (DL_FUNC)&references, 1},
    {"repository", (DL_FUNC)&repository, 1},
    {"remotes", (DL_FUNC)&remotes, 1},
    {"remote_url", (DL_FUNC)&remote_url, 2},
    {"revisions", (DL_FUNC)&revisions, 1},
    {"state", (DL_FUNC)&state, 1},
    {"tags", (DL_FUNC)&tags, 1},
    {"workdir", (DL_FUNC)&workdir, 1},
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
