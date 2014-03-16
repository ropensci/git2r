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

/** @file git2r.c
 *  @brief R bindings to the libgit2 library
 *
 *  These functions are called from R with .Call to interface the
 *  libgit2 library from R.
 *
 */

#include <Rdefines.h>
#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>

#include <git2.h>
#include <git2/repository.h>

static void init_reference(git_reference *ref, SEXP reference);
static git_repository* get_repository(const SEXP repo);
static void init_signature(const git_signature *sig, SEXP signature);
static int number_of_branches(git_repository *repo, int flags, size_t *n);

/**
 * Error messages
 */

const char err_alloc_memory_buffer[] = "Unable to allocate memory buffer";
const char err_invalid_repository[] = "Invalid repository";
const char err_unexpected_type_of_branch[] = "Unexpected type of branch";
const char err_unexpected_head_of_branch[] = "Unexpected head of branch";

/**
 * Add files to a repository
 *
 * @param repo S4 class git_repository
 * @param path
 * @return R_NilValue
 */
SEXP add(const SEXP repo, const SEXP path)
{
    int err;
    git_index *index = NULL;
    git_repository *repository = NULL;

    if (R_NilValue == path)
        error("'path' equals R_NilValue");
    if (!isString(path))
        error("'path' must be a string");

    repository= get_repository(repo);
    if (!repository)
        error(err_invalid_repository);

    err = git_repository_index(&index, repository);
    if (err < 0)
        goto cleanup;

    err = git_index_add_bypath(index, CHAR(STRING_ELT(path, 0)));
    if (err < 0)
        goto cleanup;

    err = git_index_write(index);
    if (err < 0)
        goto cleanup;

cleanup:
    if (index)
        git_index_free(index);

    if (repository)
        git_repository_free(repository);

    if (err < 0) {
        const git_error *e = giterr_last();
        error("Error %d/%d: %s\n", err, e->klass, e->message);
    }

    return R_NilValue;
}

/**
 * List branches in a repository
 *
 * @param repo S4 class git_repository
 * @return VECXSP with S4 objects of class git_branch
 */
SEXP branches(const SEXP repo, const SEXP flags)
{
    SEXP list;
    int err = 0;
    const char* err_msg = NULL;
    git_branch_iterator *iter = NULL;
    size_t i = 0, n = 0;
    size_t protected = 0;
    git_repository *repository = NULL;

    repository = get_repository(repo);
    if (!repository)
        error(err_invalid_repository);

    /* Count number of branches before creating the list */
    err = number_of_branches(repository, INTEGER(flags)[0], &n);
    if (err < 0)
        goto cleanup;

    PROTECT(list = allocVector(VECSXP, n));
    protected++;

    err = git_branch_iterator_new(&iter, repository,  INTEGER(flags)[0]);
    if (err < 0)
        goto cleanup;

    for (;;) {
        SEXP branch;
        git_branch_t type;
        git_reference *ref;
        const char *refname;

        err = git_branch_next(&ref, &type, iter);
        if (err < 0) {
            if (GIT_ITEROVER == err) {
                err = 0;
                break;
            }
            goto cleanup;
        }

        PROTECT(branch = NEW_OBJECT(MAKE_CLASS("git_branch")));
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

            buf_size = git_branch_remote_name(NULL, 0, repository, refname);
            buf = malloc(buf_size * sizeof(char));
            if (NULL == buf) {
                err = -1;
                err_msg = err_alloc_memory_buffer;
                goto cleanup;
            }

            git_branch_remote_name(buf, buf_size, repository, refname);
            SET_SLOT(branch, Rf_install("remote"), ScalarString(mkChar(buf)));

            err = git_remote_load(&remote, repository, buf);
            if (err < 0) {
                err = git_remote_create_inmemory(&remote, repository, NULL, buf);
                if (err < 0)
                    goto cleanup;
            }

            SET_SLOT(branch,
                     Rf_install("url"),
                     ScalarString(mkChar(git_remote_url(remote))));

            free(buf);
            git_remote_free(remote);
            break;
        }
        default:
            err = -1;
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
            err = -1;
            err_msg = err_unexpected_head_of_branch;
            goto cleanup;
        }

        git_reference_free(ref);
        SET_VECTOR_ELT(list, i, branch);
        UNPROTECT(1);
        protected--;
        i++;
    }

cleanup:
    if (iter)
        git_branch_iterator_free(iter);

    if (repository)
        git_repository_free(repository);

    if (protected)
        UNPROTECT(protected);

    if (err < 0) {
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
 * Commit
 *
 * @param repo S4 class git_repository
 * @param message
 * @param author S4 class git_signature
 * @param committer S4 class git_signature
 * @param parent_list
 * @return R_NilValue
 */
SEXP commit(SEXP repo, SEXP message, SEXP author, SEXP committer, SEXP parent_list)
{
    SEXP when;
    int err;
    const char* err_msg = NULL;
    git_signature *sig_author = NULL;
    git_signature *sig_committer = NULL;
    git_index *index = NULL;
    git_oid commit_id, tree_oid;
    git_repository *repository = NULL;
    git_tree *tree = NULL;
    size_t i, parent_count;
    git_commit **parents = NULL;

    if (R_NilValue == repo
        || R_NilValue == message
        || !isString(message)
        || R_NilValue == author
        || S4SXP != TYPEOF(author)
        || R_NilValue == committer
        || S4SXP != TYPEOF(committer)
        || R_NilValue == parent_list
        || !isString(parent_list))
        error("Invalid arguments to commit");

    if (0 != strcmp(CHAR(STRING_ELT(getAttrib(author, R_ClassSymbol), 0)),
                    "git_signature"))
        error("author argument not a git_signature");

    if (0 != strcmp(CHAR(STRING_ELT(getAttrib(committer, R_ClassSymbol), 0)),
                    "git_signature"))
        error("committer argument not a git_signature");

    repository = get_repository(repo);
    if (!repository)
        error(err_invalid_repository);

    when = GET_SLOT(author, Rf_install("when"));
    err = git_signature_new(&sig_author,
                            CHAR(STRING_ELT(GET_SLOT(author, Rf_install("name")), 0)),
                            CHAR(STRING_ELT(GET_SLOT(author, Rf_install("email")), 0)),
                            REAL(GET_SLOT(when, Rf_install("time")))[0],
                            REAL(GET_SLOT(when, Rf_install("offset")))[0]);
    if (err < 0)
        goto cleanup;

    when = GET_SLOT(committer, Rf_install("when"));
    err = git_signature_new(&sig_committer,
                            CHAR(STRING_ELT(GET_SLOT(committer, Rf_install("name")), 0)),
                            CHAR(STRING_ELT(GET_SLOT(committer, Rf_install("email")), 0)),
                            REAL(GET_SLOT(when, Rf_install("time")))[0],
                            REAL(GET_SLOT(when, Rf_install("offset")))[0]);
    if (err < 0)
        goto cleanup;

    err = git_repository_index(&index, repository);
    if (err < 0)
        goto cleanup;

    err = git_index_write_tree(&tree_oid, index);
    if (err < 0)
        goto cleanup;

    err = git_tree_lookup(&tree, repository, &tree_oid);
    if (err < 0)
        goto cleanup;

    parent_count = LENGTH(parent_list);
    if (parent_count) {
        parents = calloc(parent_count, sizeof(git_commit*));
        if (NULL == parents) {
            err = -1;
            err_msg = err_alloc_memory_buffer;
            goto cleanup;
        }

        for (i = 0; i < parent_count; i++) {
            git_oid oid;

            err = git_oid_fromstr(&oid, CHAR(STRING_ELT(parent_list, 0)));
            if (err < 0)
                goto cleanup;

            err = git_commit_lookup(&parents[i], repository, &oid);
            if (err < 0)
                goto cleanup;
        }
    }

    err = git_commit_create(&commit_id,
                            repository,
                            "HEAD",
                            sig_author,
                            sig_committer,
                            NULL,
                            CHAR(STRING_ELT(message, 0)),
                            tree,
                            parent_count,
                            (const git_commit**)parents);
    if (err < 0)
        goto cleanup;

cleanup:
    if (sig_author)
        git_signature_free(sig_author);

    if (sig_committer)
        git_signature_free(sig_committer);

    if (index)
        git_index_free(index);

    if (tree)
        git_tree_free(tree);

    if (repository)
        git_repository_free(repository);

    if (parents) {
        for (i = 0; i < parent_count; i++) {
            if (parents[i])
                git_commit_free(parents[i]);
        }
        free(parents);
    }

    if (err < 0) {
        if (err_msg) {
            error(err_msg);
        } else {
            const git_error *e = giterr_last();
            error("Error %d/%d: %s\n", err, e->klass, e->message);
        }
    }

    return R_NilValue;
}

/**
 * Get the configured signature for a repository
 *
 * @param repo S4 class git_repository
 * @return S4 class git_signature
 */
SEXP default_signature(const SEXP repo)
{
    int err;
    git_repository *repository = NULL;
    git_signature *signature = NULL;
    SEXP sig;

    repository = get_repository(repo);
    if (!repository)
        error(err_invalid_repository);

    err = git_signature_default(&signature, repository);
    if (err < 0)
        goto cleanup;

    PROTECT(sig = NEW_OBJECT(MAKE_CLASS("git_signature")));
    init_signature(signature, sig);

cleanup:
    if (repository)
        git_repository_free(repository);

    if (signature)
        git_signature_free(signature);

    UNPROTECT(1);

    if (err < 0) {
        const git_error *e = giterr_last();
        error("Error %d/%d: %s\n", err, e->klass, e->message);
    }

    return sig;
}

/**
 * Get repo slot from S4 class git_repository
 *
 * @param repo S4 class git_repository
 * @return a git_repository pointer on success else NULL
 */
static git_repository* get_repository(const SEXP repo)
{
    SEXP class_name;
    SEXP path;
    git_repository *r;

    if (R_NilValue == repo || S4SXP != TYPEOF(repo))
        return NULL;

    class_name = getAttrib(repo, R_ClassSymbol);
    if (0 != strcmp(CHAR(STRING_ELT(class_name, 0)), "git_repository"))
        return NULL;

    path = GET_SLOT(repo, Rf_install("path"));
    if (R_NilValue == path)
        return NULL;

    if (git_repository_open(&r, CHAR(STRING_ELT(path, 0))) < 0)
        return NULL;

    return r;
}

/**
 * Init a repository.
 *
 * @param path
 * @param bare
 * @return R_NilValue
 */
SEXP init(const SEXP path, const SEXP bare)
{
    int err;
    git_repository *repository = NULL;

    if (R_NilValue == path)
        error("'path' equals R_NilValue");
    if (!isString(path))
        error("'path' must be a string");
    if (R_NilValue == bare)
        error("'bare' equals R_NilValue");
    if (!isLogical(bare))
        error("'bare' must be a logical");

    err = git_repository_init(&repository,
                              CHAR(STRING_ELT(path, 0)),
                              LOGICAL(bare)[0]);
    if (err < 0)
        error("Unable to init repository");

    git_repository_free(repository);

    return R_NilValue;
}

/**
 * Init slots in S4 class git_commit
 *
 * @param commit a commit object
 * @param sexp_commit S4 class git_commit
 * @return void
 */
static void init_commit(const git_commit *commit, SEXP sexp_commit)
{
    const char *message;
    const char *summary;
    const git_signature *author;
    const git_signature *committer;

    author = git_commit_author(commit);
    if (author) {
        SEXP sexp_author;

        PROTECT(sexp_author = NEW_OBJECT(MAKE_CLASS("git_signature")));
        init_signature(author, sexp_author);
        SET_SLOT(sexp_commit, Rf_install("author"), sexp_author);
        UNPROTECT(1);
    }

    committer = git_commit_committer(commit);
    if (committer) {
        SEXP sexp_committer;

        PROTECT(sexp_committer = NEW_OBJECT(MAKE_CLASS("git_signature")));
        init_signature(committer, sexp_committer);
        SET_SLOT(sexp_commit, Rf_install("committer"), sexp_committer);
        UNPROTECT(1);
    }

    summary = git_commit_summary(commit);
    if (summary) {
        SET_SLOT(sexp_commit,
                 Rf_install("summary"),
                 ScalarString(mkChar(summary)));
    }

    message = git_commit_message(commit);
    if (message) {
        SET_SLOT(sexp_commit,
                 Rf_install("message"),
                 ScalarString(mkChar(message)));
    }
}

/**
 * Init slots in S4 class git_reference.
 *
 * @param ref
 * @param reference
 * @return void
 */
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

/**
 * Init slots in S4 class git_signature.
 *
 * @param sig
 * @param signature
 * @return void
 */
static void init_signature(const git_signature *sig, SEXP signature)
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
 * Check if repository is bare.
 *
 * @param repo S4 class git_repository
 * @return
 */
SEXP is_bare(const SEXP repo)
{
    SEXP result;
    git_repository *repository;

    repository= get_repository(repo);
    if (!repository)
        error(err_invalid_repository);

    if (git_repository_is_bare(repository))
        result = ScalarLogical(TRUE);
    else
        result = ScalarLogical(FALSE);

    git_repository_free(repository);

    return result;
}

/**
 * Check if repository is empty.
 *
 * @param repo S4 class git_repository
 * @return
 */
SEXP is_empty(const SEXP repo)
{
    SEXP result;
    git_repository *repository;

    repository= get_repository(repo);
    if (!repository)
        error(err_invalid_repository);

    if (git_repository_is_empty(repository))
        result = ScalarLogical(TRUE);
    else
        result = ScalarLogical(FALSE);

    git_repository_free(repository);

    return result;
}

/**
 * Check if valid repository.
 *
 * @param path
 * @return
 */
SEXP is_repository(const SEXP path)
{
    SEXP result;
    git_repository *repository;
    int err;

    if (R_NilValue == path)
        error("'path' equals R_NilValue");
    if (!isString(path))
        error("'path' must be a string");

    err = git_repository_open(&repository, CHAR(STRING_ELT(path, 0)));
    if (err < 0) {
        result = ScalarLogical(FALSE);
    } else {
        git_repository_free(repository);
        result = ScalarLogical(TRUE);
    }

    return result;
}

/**
 * Count number of branches.
 *
 * @param repo S4 class git_repository
 * @param flags
 * @return
 */
static int number_of_branches(git_repository *repo, int flags, size_t *n)
{
    int err;
    git_branch_iterator *iter;
    git_branch_t type;
    git_reference *ref;

    *n = 0;

    err = git_branch_iterator_new(&iter, repo, flags);
    if (err < 0)
        return err;

    for (;;) {
        err = git_branch_next(&ref, &type, iter);
        if (err < 0)
            break;
        git_reference_free(ref);
        (*n)++;
    }

    git_branch_iterator_free(iter);

    if (GIT_ITEROVER != err)
        return err;
    return 0;
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

    while (!git_revwalk_next(&oid, walker))
        n++;
    return n;
}

/**
 * Get all references that can be found in a repository.
 *
 * @param repo S4 class git_repository
 * @return VECXSP with S4 objects of class git_reference
 */
SEXP references(const SEXP repo)
{
    int i, err;
    git_strarray ref_list;
    SEXP list = R_NilValue;
    SEXP names = R_NilValue;
    git_reference *ref;
    git_repository *repository;

    repository = get_repository(repo);
    if (!repository)
        error(err_invalid_repository);

    err = git_reference_list(&ref_list, repository);
    if (err < 0)
        goto cleanup;

    PROTECT(list = allocVector(VECSXP, ref_list.count));
    PROTECT(names = allocVector(STRSXP, ref_list.count));

    for (i = 0; i < ref_list.count; i++) {
        SEXP reference;

        err = git_reference_lookup(&ref, repository, ref_list.strings[i]);
        if (err < 0)
            goto cleanup;

        PROTECT(reference = NEW_OBJECT(MAKE_CLASS("git_reference")));
        init_reference(ref, reference);
        SET_STRING_ELT(names, i, mkChar(ref_list.strings[i]));
        SET_VECTOR_ELT(list, i, reference);
        UNPROTECT(1);
    }

cleanup:
    git_strarray_free(&ref_list);

    if (repository)
        git_repository_free(repository);

    if (R_NilValue != list && R_NilValue != names) {
        setAttrib(list, R_NamesSymbol, names);
        UNPROTECT(2);
    }

    if (err < 0) {
        const git_error *e = giterr_last();
        error("Error %d/%d: %s\n", err, e->klass, e->message);
    }

    return list;
}

/**
 * Get the configured remotes for a repo
 *
 * @param repo S4 class git_repository
 * @return
 */
SEXP remotes(const SEXP repo)
{
    int i, err;
    git_strarray rem_list;
    SEXP list;
    git_repository *repository;

    repository = get_repository(repo);
    if (!repository)
        error(err_invalid_repository);

    err = git_remote_list(&rem_list, repository);
    if (err < 0)
        goto cleanup;

    PROTECT(list = allocVector(STRSXP, rem_list.count));
    for (i = 0; i < rem_list.count; i++)
        SET_STRING_ELT(list, i, mkChar(rem_list.strings[i]));
    UNPROTECT(1);

cleanup:
    git_strarray_free(&rem_list);

    if (repository)
        git_repository_free(repository);

    if (err < 0) {
        const git_error *e = giterr_last();
        error("Error %d/%d: %s\n", err, e->klass, e->message);
    }

    return list;
}

/**
 * Get the remote's url
 *
 * @param repo S4 class git_repository
 * @return
 */
SEXP remote_url(const SEXP repo, const SEXP remote)
{
    int err;
    SEXP url;
    size_t len = LENGTH(remote);
    size_t i = 0;
    git_repository *repository;

    repository = get_repository(repo);
    if (!repository)
        error(err_invalid_repository);

    PROTECT(url = allocVector(STRSXP, len));

    for (; i < len; i++) {
        git_remote *r;

        err = git_remote_load(&r, repository, CHAR(STRING_ELT(remote, i)));
        if (err < 0)
            goto cleanup;

        SET_STRING_ELT(url, i, mkChar(git_remote_url(r)));
        git_remote_free(r);
    }

cleanup:
    git_repository_free(repository);

    UNPROTECT(1);

    if (err < 0) {
        const git_error *e = giterr_last();
        error("Error %d/%d: %s\n", err, e->klass, e->message);
    }

    return url;
}

/**
 * List revisions
 *
 * @param repo S4 class git_repository
 * @return
 */
SEXP revisions(SEXP repo)
{
    int i=0;
    int err = 0;
    SEXP list;
    size_t n = 0;
    git_revwalk *walker = NULL;
    git_repository *repository;

    repository = get_repository(repo);
    if (!repository)
        error(err_invalid_repository);

    err = git_revwalk_new(&walker, repository);
    if (err < 0)
        goto cleanup;

    err = git_revwalk_push_head(walker);
    if (err < 0)
        goto cleanup;

    /* Count number of revisions before creating the list */
    n = number_of_revisions(walker);

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
        init_commit(commit, sexp_commit);
        SET_VECTOR_ELT(list, i, sexp_commit);
        UNPROTECT(1);
        i++;

        git_commit_free(commit);
    }

cleanup:
    if (walker)
        git_revwalk_free(walker);

    git_repository_free(repository);

    UNPROTECT(1);

    if (err < 0) {
        const git_error *e = giterr_last();
        error("Error %d/%d: %s\n", err, e->klass, e->message);
    }

    return list;
}

/**
 * Get state of repository.
 *
 * @param repo S4 class git_repository
 * @return
 */
SEXP state(const SEXP repo)
{
    SEXP result;
    git_repository *repository;

    repository = get_repository(repo);
    if (!repository)
        error(err_invalid_repository);

    result = ScalarInteger(git_repository_state(repository));

    git_repository_free(repository);

    return result;
}

/**
 * Get all tags that can be found in a repository.
 *
 * @param repo S4 class git_repository
 * @return VECXSP with S4 objects of class tag
 */
SEXP tags(const SEXP repo)
{
    int i, err;
    const char* err_msg = NULL;
    git_strarray tag_list;
    SEXP list;
    SEXP names;
    size_t protected = 0;
    const git_signature *signature;
    git_repository *repository;

    repository = get_repository(repo);
    if (!repository)
        error(err_invalid_repository);

    err = git_tag_list(&tag_list, repository);
    if (err < 0)
        goto cleanup;

    PROTECT(list = allocVector(VECSXP, tag_list.count));
    PROTECT(names = allocVector(STRSXP, tag_list.count));
    protected = 2;

    for (i = 0; i < tag_list.count; i++) {
        char* buf;
        git_reference *ref;
        SEXP tag;
        char *tagname;
        size_t tagname_len;

        PROTECT(tag = NEW_OBJECT(MAKE_CLASS("git_tag")));
        protected++;

        tagname = tag_list.strings[i];

        /* Prefix tagname with "refs/tags/" to lookup reference */
        tagname_len = strlen(tagname);
        buf = malloc((tagname_len+11)*sizeof(char));
        if (!buf) {
            err = -1;
            err_msg = err_alloc_memory_buffer;
            goto cleanup;
        }
        *buf = '\0';
        strncat(buf, "refs/tags/", 10);
        strncat(buf, tagname, tagname_len);
        err = git_reference_lookup(&ref, repository, buf);
        if (err < 0) {
            free(buf);
            goto cleanup;
        }
        free(buf);
        init_reference(ref, tag);

        /* Fill in signature for tag */
        /* oid = git_reference_target(ref); */
        /* if (oid) { */
        /*     err = git_tag_lookup(&t, repository, oid); */
        /*     signature = git_tag_tagger(t); */
        /*     SET_SLOT(GET_SLOT(tag, Rf_install("sig")), */
        /*              Rf_install("name"), */
        /*              ScalarString(mkChar(signature->name))); */
        /* } */

        SET_STRING_ELT(names, i, mkChar(tagname));
        SET_VECTOR_ELT(list, i, tag);
        UNPROTECT(1);
        protected--;
    }

cleanup:
    git_strarray_free(&tag_list);
    git_repository_free(repository);

    setAttrib(list, R_NamesSymbol, names);
    if (protected)
        UNPROTECT(protected);

    if (err < 0) {
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
 * Get workdir of repository.
 *
 * @param repo S4 class git_repository
 * @return
 */
SEXP workdir(const SEXP repo)
{
    SEXP result;
    git_repository *repository;

    repository = get_repository(repo);
    if (!repository)
        error(err_invalid_repository);

    result = ScalarString(mkChar(git_repository_workdir(repository)));

    git_repository_free(repository);

    return result;
}

static const R_CallMethodDef callMethods[] =
{
    {"add", (DL_FUNC)&add, 2},
    {"branches", (DL_FUNC)&branches, 2},
    {"commit", (DL_FUNC)&commit, 5},
    {"default_signature", (DL_FUNC)&default_signature, 1},
    {"init", (DL_FUNC)&init, 2},
    {"is_bare", (DL_FUNC)&is_bare, 1},
    {"is_empty", (DL_FUNC)&is_empty, 1},
    {"is_repository", (DL_FUNC)&is_repository, 1},
    {"references", (DL_FUNC)&references, 1},
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
