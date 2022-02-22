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
#include "git2r_arg.h"
#include "git2r_deprecated.h"
#include "git2r_diff.h"
#include "git2r_error.h"
#include "git2r_repository.h"
#include "git2r_S3.h"
#include "git2r_tree.h"

#include <git2.h>
#include <git2/sys/diff.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

static int
git2r_diff_count(
    git_diff *diff, size_t *num_files,
    size_t *max_hunks, size_t *max_lines);

static int
git2r_diff_format_to_r(
    git_diff *diff,
    SEXP dest);

SEXP git2r_diff_index_to_wd(
    SEXP repo,
    SEXP filename,
    git_diff_options *opts);

SEXP
git2r_diff_head_to_index(
    SEXP repo,
    SEXP filename,
    git_diff_options *opts);

SEXP
git2r_diff_tree_to_wd(
    SEXP tree,
    SEXP filename,
    git_diff_options *opts);

SEXP
git2r_diff_tree_to_index(
    SEXP tree,
    SEXP filename,
    git_diff_options *opts);

SEXP
git2r_diff_tree_to_tree(
    SEXP tree1,
    SEXP tree2,
    SEXP filename,
    git_diff_options *opts);

/**
 * Diff
 *
 * Setting index to TRUE is essentially like supplying the --cached
 * option to command line git.
 *
 * - If tree1 is NULL and index is FALSE, then we compare the working
 *   directory to the index. (tree2 must be NULL in this case.)
 * - If tree1 is NULL and index is TRUE, then we compare the index
 *   to HEAD. (tree2 must be NULL in this case.)
 * - If tree1 is not NULL and tree2 is NULL, and index is FALSE,
 *   then we compare the working directory to the tree1.
 *   (repo must be NULL in this case.)
 * - If tree1 is not NULL and tree2 is NULL, and index is TRUE,
 *   then we compare the index to tree1. (repo must be NULL.)
 * - If tree1 is not NULL and tree2 is not NULL, then we compare
 *   tree1 to tree2. (repo must be NULL, index is ignored in this case).
 *
 * @param repo Repository.
 * @param tree1 The first tree to compare.
 * @param tree2 The second tree to compare.
 * @param index Whether to compare to the index.
 * @param filename Determines where to write the diff. If filename is
 * R_NilValue, then the diff is written to a S3 class git_diff
 * object. If filename is a character vector of length 0, then the
 * diff is written to a character vector. If filename is a character
 * vector of length one with non-NA value, the diff is written to a
 * file with name filename (the file is overwritten if it exists).
 * @param context_lines The number of unchanged lines that define the
 * boundary of a hunk (and to display before and after).
 * @param interhunk_lines The maximum number of unchanged lines
 * between hunk boundaries before the hunks will be merged into one.
 * @param old_prefix The virtual "directory" prefix for old file
 * names in hunk headers.
 * @param new_prefix The virtual "directory" prefix for new file
 * names in hunk headers.
 * @param id_abbrev The abbreviation length to use when formatting
 * object ids. Defaults to the value of 'core.abbrev' from the
 * config, or 7 if NULL.
 * @param path A character vector of paths / fnmatch patterns to
 *     constrain diff. Default is NULL which include all paths.
 * @param max_size A size (in bytes) above which a blob will be
 * marked as binary automatically; pass a negative value to
 * disable. Defaults to 512MB when max_size is NULL.
 * @return A S3 class git_diff object if filename equals R_NilValue. A
 * character vector with diff if filename has length 0. Oterwise NULL.
 */
SEXP attribute_hidden
git2r_diff(
    SEXP repo,
    SEXP tree1,
    SEXP tree2,
    SEXP index,
    SEXP filename,
    SEXP context_lines,
    SEXP interhunk_lines,
    SEXP old_prefix,
    SEXP new_prefix,
    SEXP id_abbrev,
    SEXP path,
    SEXP max_size)
{
    int c_index;
    git_diff_options opts = GIT_DIFF_OPTIONS_INIT;

    if (git2r_arg_check_logical(index))
        git2r_error(__func__, NULL, "'index'", git2r_err_logical_arg);
    c_index = LOGICAL(index)[0];

    if (git2r_arg_check_integer_gte_zero(context_lines))
        git2r_error(__func__, NULL, "'context_lines'", git2r_err_integer_gte_zero_arg);
    opts.context_lines = INTEGER(context_lines)[0];

    if (git2r_arg_check_integer_gte_zero(interhunk_lines))
        git2r_error(__func__, NULL, "'interhunk_lines'", git2r_err_integer_gte_zero_arg);
    opts.interhunk_lines = INTEGER(interhunk_lines)[0];

    if (git2r_arg_check_string(old_prefix))
        git2r_error(__func__, NULL, "'old_prefix'", git2r_err_string_arg);
    opts.old_prefix = CHAR(STRING_ELT(old_prefix, 0));

    if (git2r_arg_check_string(new_prefix))
        git2r_error(__func__, NULL, "'new_prefix'", git2r_err_string_arg);
    opts.new_prefix = CHAR(STRING_ELT(new_prefix, 0));

    if (!Rf_isNull(id_abbrev)) {
        if (git2r_arg_check_integer_gte_zero(id_abbrev))
            git2r_error(__func__, NULL, "'id_abbrev'",
                        git2r_err_integer_gte_zero_arg);
        opts.id_abbrev = INTEGER(id_abbrev)[0];
    }

    if (!Rf_isNull(path)) {
        int error;

        if (git2r_arg_check_string_vec(path))
            git2r_error(__func__, NULL, "'path'", git2r_err_string_vec_arg);

        error = git2r_copy_string_vec(&(opts.pathspec), path);
        if (error || !opts.pathspec.count) {
            free(opts.pathspec.strings);
            git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);
        }
    }

    if (!Rf_isNull(max_size)) {
        if (git2r_arg_check_integer(max_size))
            git2r_error(__func__, NULL, "'max_size'", git2r_err_integer_arg);
        opts.max_size = INTEGER(max_size)[0];
    }

    if (Rf_isNull(tree1) && ! c_index) {
	if (!Rf_isNull(tree2))
	    git2r_error(__func__, NULL, git2r_err_diff_arg, NULL);
	return git2r_diff_index_to_wd(repo, filename, &opts);
    }

    if (Rf_isNull(tree1) && c_index) {
	if (!Rf_isNull(tree2))
	    git2r_error(__func__, NULL, git2r_err_diff_arg, NULL);
	return git2r_diff_head_to_index(repo, filename, &opts);
    }

    if (!Rf_isNull(tree1) && Rf_isNull(tree2) && !c_index) {
	if (!Rf_isNull(repo))
	    git2r_error(__func__, NULL, git2r_err_diff_arg, NULL);
	return git2r_diff_tree_to_wd(tree1, filename, &opts);
    }

    if (!Rf_isNull(tree1) && Rf_isNull(tree2) && c_index) {
	if (!Rf_isNull(repo))
	    git2r_error(__func__, NULL, git2r_err_diff_arg, NULL);
	return git2r_diff_tree_to_index(tree1, filename, &opts);
    }

    if (!Rf_isNull(repo))
        git2r_error(__func__, NULL, git2r_err_diff_arg, NULL);
    return git2r_diff_tree_to_tree(tree1, tree2, filename, &opts);
}

static int
git2r_diff_print_cb(
    const git_diff_delta *delta,
    const git_diff_hunk *hunk,
    const git_diff_line *line,
    void *payload)
{
    int error;

    GIT2R_UNUSED(delta);
    GIT2R_UNUSED(hunk);

    if (line->origin == GIT_DIFF_LINE_CONTEXT ||
        line->origin == GIT_DIFF_LINE_ADDITION ||
        line->origin == GIT_DIFF_LINE_DELETION) {
        while ((error = fputc(line->origin, (FILE *)payload)) == EINTR)
            continue;
        if (error == EOF)
            return -1;
    }

    if (fwrite(line->content, line->content_len, 1, (FILE *)payload) != 1)
        return -1;

    return 0;
}

/**
 * Create a diff between the repository index and the workdir
 * directory.
 *
 * @param repo S3 class git_repository
 * @param filename Determines where to write the diff. If filename is
 * R_NilValue, then the diff is written to a S3 class git_diff
 * object. If filename is a character vector of length 0, then the
 * diff is written to a character vector. If filename is a character
 * vector of length one with non-NA value, the diff is written to a
 * file with name filename (the file is overwritten if it exists).
 * @param opts Structure describing options about how the diff
 * should be executed.
 * @return A S3 class git_diff object if filename equals R_NilValue. A
 * character vector with diff if filename has length 0. Oterwise NULL.
 */
SEXP attribute_hidden
git2r_diff_index_to_wd(
    SEXP repo,
    SEXP filename,
    git_diff_options *opts)
{
    int error, nprotect = 0;
    git_repository *repository = NULL;
    git_diff *diff = NULL;
    SEXP result = R_NilValue;

    if (git2r_arg_check_filename(filename))
        git2r_error(__func__, NULL, "'filename'", git2r_err_filename_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git_diff_index_to_workdir(&diff,
                                      repository,
                                      /*index=*/ NULL,
                                      opts);

    if (error)
	goto cleanup;

    if (Rf_isNull(filename)) {
        PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_diff));
        nprotect++;
        Rf_setAttrib(result, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_diff));

        SET_VECTOR_ELT(result, git2r_S3_item__git_diff__old, Rf_mkString("index"));

        SET_VECTOR_ELT(result, git2r_S3_item__git_diff__new, Rf_mkString("workdir"));

        error = git2r_diff_format_to_r(diff, result);
    } else if (0 == Rf_length(filename)) {
        git_buf buf = GIT_BUF_INIT_CONST(NULL, 0);

        error = git_diff_to_buf(&buf, diff, GIT_DIFF_FORMAT_PATCH);
        if (!error) {
            PROTECT(result = Rf_mkString(buf.ptr));
            nprotect++;
        }

        GIT2R_BUF_DISPOSE(&buf);
    } else {
        FILE *fp = fopen(CHAR(STRING_ELT(filename, 0)), "w+");

        error = git_diff_print(
            diff,
            GIT_DIFF_FORMAT_PATCH,
            git2r_diff_print_cb,
            fp);

        if (fp)
            fclose(fp);
    }

cleanup:
    free(opts->pathspec.strings);
    git_diff_free(diff);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Create a diff between head and repository index
 *
 * @param repo S3 class git_repository
 * @param filename Determines where to write the diff. If filename is
 * R_NilValue, then the diff is written to a S3 class git_diff
 * object. If filename is a character vector of length 0, then the
 * diff is written to a character vector. If filename is a character
 * vector of length one with non-NA value, the diff is written to a
 * file with name filename (the file is overwritten if it exists).
 * @param opts Structure describing options about how the diff
 * should be executed.
 * @return A S3 class git_diff object if filename equals R_NilValue. A
 * character vector with diff if filename has length 0. Oterwise NULL.
 */
SEXP attribute_hidden
git2r_diff_head_to_index(
    SEXP repo,
    SEXP filename,
    git_diff_options *opts)
{
    int error, nprotect = 0;
    git_repository *repository = NULL;
    git_diff *diff = NULL;
    git_object *obj = NULL;
    git_tree *head = NULL;
    SEXP result = R_NilValue;

    if (git2r_arg_check_filename(filename))
        git2r_error(__func__, NULL, "'filename'", git2r_err_filename_arg);

    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    error = git_revparse_single(&obj, repository, "HEAD^{tree}");
    if (error)
	goto cleanup;

    error = git_tree_lookup(&head, repository, git_object_id(obj));
    if (error)
	goto cleanup;

    error = git_diff_tree_to_index(
        &diff,
        repository,
        head,
        /* index= */ NULL,
        opts);
    if (error)
	goto cleanup;

    if (Rf_isNull(filename)) {
        /* TODO: object instead of HEAD string */
        PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_diff));
        nprotect++;
        Rf_setAttrib(result, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_diff));

        SET_VECTOR_ELT(
            result,
            git2r_S3_item__git_diff__old,
            Rf_mkString("HEAD"));

        SET_VECTOR_ELT(
            result,
            git2r_S3_item__git_diff__new,
            Rf_mkString("index"));

        error = git2r_diff_format_to_r(diff, result);
    } else if (0 == Rf_length(filename)) {
        git_buf buf = GIT_BUF_INIT_CONST(NULL, 0);

        error = git_diff_to_buf(
            &buf,
            diff,
            GIT_DIFF_FORMAT_PATCH);
        if (!error) {
            PROTECT(result = Rf_mkString(buf.ptr));
            nprotect++;
        }

        GIT2R_BUF_DISPOSE(&buf);
    } else {
        FILE *fp = fopen(CHAR(STRING_ELT(filename, 0)), "w+");

        error = git_diff_print(
            diff,
            GIT_DIFF_FORMAT_PATCH,
            git_diff_print_callback__to_file_handle,
            fp);

        if (fp)
            fclose(fp);
    }

cleanup:
    free(opts->pathspec.strings);
    git_tree_free(head);
    git_object_free(obj);
    git_diff_free(diff);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Create a diff between a tree and the working directory
 *
 * @param tree S3 class git_tree
 * @param filename Determines where to write the diff. If filename is
 * R_NilValue, then the diff is written to a S3 class git_diff
 * object. If filename is a character vector of length 0, then the
 * diff is written to a character vector. If filename is a character
 * vector of length one with non-NA value, the diff is written to a
 * file with name filename (the file is overwritten if it exists).
 * @param opts Structure describing options about how the diff
 * should be executed.
 * @return A S3 class git_diff object if filename equals R_NilValue. A
 * character vector with diff if filename has length 0. Oterwise NULL.
 */
SEXP attribute_hidden
git2r_diff_tree_to_wd(
    SEXP tree,
    SEXP filename,
    git_diff_options *opts)
{
    int error, nprotect = 0;
    git_repository *repository = NULL;
    git_diff *diff = NULL;
    git_object *obj = NULL;
    git_tree *c_tree = NULL;
    SEXP sha;
    SEXP result = R_NilValue;
    SEXP repo;

    if (git2r_arg_check_tree(tree))
        git2r_error(__func__, NULL, "'tree'", git2r_err_tree_arg);
    if (git2r_arg_check_filename(filename))
        git2r_error(__func__, NULL, "'filename'", git2r_err_filename_arg);

    repo = git2r_get_list_element(tree, "repo");
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    sha = git2r_get_list_element(tree, "sha");
    error = git_revparse_single(&obj, repository, CHAR(STRING_ELT(sha, 0)));
    if (error)
	goto cleanup;

    error = git_tree_lookup(&c_tree, repository, git_object_id(obj));
    if (error)
	goto cleanup;

    error = git_diff_tree_to_workdir(&diff, repository, c_tree, opts);
    if (error)
	goto cleanup;

    if (Rf_isNull(filename)) {
        PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_diff));
        nprotect++;
        Rf_setAttrib(result, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_diff));

        SET_VECTOR_ELT(
            result,
            git2r_S3_item__git_diff__old,
            tree);

        SET_VECTOR_ELT(
            result,
            git2r_S3_item__git_diff__new,
            Rf_mkString("workdir"));

        error = git2r_diff_format_to_r(diff, result);
    } else if (0 == Rf_length(filename)) {
        git_buf buf = GIT_BUF_INIT_CONST(NULL, 0);

        error = git_diff_to_buf(
            &buf,
            diff,
            GIT_DIFF_FORMAT_PATCH);
        if (!error) {
            PROTECT(result = Rf_mkString(buf.ptr));
            nprotect++;
        }

        GIT2R_BUF_DISPOSE(&buf);
    } else {
        FILE *fp = fopen(CHAR(STRING_ELT(filename, 0)), "w+");

        error = git_diff_print(
            diff,
            GIT_DIFF_FORMAT_PATCH,
            git_diff_print_callback__to_file_handle,
            fp);

        if (fp)
            fclose(fp);
    }

cleanup:
    free(opts->pathspec.strings);
    git_diff_free(diff);
    git_tree_free(c_tree);
    git_object_free(obj);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
        git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Create a diff between a tree and repository index
 *
 * @param tree S3 class git_tree
 * @param filename Determines where to write the diff. If filename is
 * R_NilValue, then the diff is written to a S3 class git_diff
 * object. If filename is a character vector of length 0, then the
 * diff is written to a character vector. If filename is a character
 * vector of length one with non-NA value, the diff is written to a
 * file with name filename (the file is overwritten if it exists).
 * @param opts Structure describing options about how the diff
 * should be executed.
 * @return A S3 class git_diff object if filename equals R_NilValue. A
 * character vector with diff if filename has length 0. Oterwise NULL.
 */
SEXP attribute_hidden
git2r_diff_tree_to_index(
    SEXP tree,
    SEXP filename,
    git_diff_options *opts)
{
    int error, nprotect = 0;
    git_repository *repository = NULL;
    git_diff *diff = NULL;
    git_object *obj = NULL;
    git_tree *c_tree = NULL;
    SEXP sha;
    SEXP result = R_NilValue;
    SEXP repo;

    if (git2r_arg_check_tree(tree))
        git2r_error(__func__, NULL, "'tree'", git2r_err_tree_arg);
    if (git2r_arg_check_filename(filename))
        git2r_error(__func__, NULL, "'filename'", git2r_err_filename_arg);

    repo = git2r_get_list_element(tree, "repo");
    repository = git2r_repository_open(repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    sha = git2r_get_list_element(tree, "sha");
    error = git_revparse_single(&obj, repository, CHAR(STRING_ELT(sha, 0)));
    if (error)
	goto cleanup;

    error = git_tree_lookup(&c_tree, repository, git_object_id(obj));
    if (error)
	goto cleanup;

    error = git_diff_tree_to_index(
        &diff,
        repository,
        c_tree,
        /* index= */ NULL,
        opts);
    if (error)
	goto cleanup;

    if (Rf_isNull(filename)) {
        PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_diff));
        nprotect++;
        Rf_setAttrib(result, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_diff));

        SET_VECTOR_ELT(
            result,
            git2r_S3_item__git_diff__old,
            tree);

        SET_VECTOR_ELT(
            result,
            git2r_S3_item__git_diff__new,
            Rf_mkString("index"));

        error = git2r_diff_format_to_r(diff, result);
    } else if (0 == Rf_length(filename)) {
        git_buf buf = GIT_BUF_INIT_CONST(NULL, 0);

        error = git_diff_to_buf(
            &buf,
            diff,
            GIT_DIFF_FORMAT_PATCH);
        if (!error) {
            PROTECT(result = Rf_mkString(buf.ptr));
            nprotect++;
        }

        GIT2R_BUF_DISPOSE(&buf);
    } else {
        FILE *fp = fopen(CHAR(STRING_ELT(filename, 0)), "w+");

        error = git_diff_print(
            diff,
            GIT_DIFF_FORMAT_PATCH,
            git_diff_print_callback__to_file_handle,
            fp);

        if (fp)
            fclose(fp);
    }

cleanup:
    free(opts->pathspec.strings);
    git_diff_free(diff);
    git_tree_free(c_tree);
    git_object_free(obj);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
	git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Create a diff with the difference between two tree objects
 *
 * @param tree1 S3 class git_tree
 * @param tree2 S3 class git_tree
 * @param filename Determines where to write the diff. If filename is
 * R_NilValue, then the diff is written to a S3 class git_diff
 * object. If filename is a character vector of length 0, then the
 * diff is written to a character vector. If filename is a character
 * vector of length one with non-NA value, the diff is written to a
 * file with name filename (the file is overwritten if it exists).
 * @param opts Structure describing options about how the diff
 * should be executed.
 * @return A S3 class git_diff object if filename equals R_NilValue. A
 * character vector with diff if filename has length 0. Oterwise NULL.
 */
SEXP attribute_hidden
git2r_diff_tree_to_tree(
    SEXP tree1,
    SEXP tree2,
    SEXP filename,
    git_diff_options *opts)
{
    int error, nprotect = 0;
    git_repository *repository = NULL;
    git_diff *diff = NULL;
    git_object *obj1 = NULL, *obj2 = NULL;
    git_tree *c_tree1 = NULL, *c_tree2 = NULL;
    SEXP result = R_NilValue;
    SEXP tree1_repo, tree2_repo;
    SEXP sha1, sha2;

    if (git2r_arg_check_tree(tree1))
        git2r_error(__func__, NULL, "'tree1'", git2r_err_tree_arg);
    if (git2r_arg_check_tree(tree2))
        git2r_error(__func__, NULL, "'tree2'", git2r_err_tree_arg);
    if (git2r_arg_check_filename(filename))
        git2r_error(__func__, NULL, "'filename'", git2r_err_filename_arg);

    tree1_repo = git2r_get_list_element(tree1, "repo");
    tree2_repo = git2r_get_list_element(tree2, "repo");
    if (git2r_arg_check_same_repo(tree1_repo, tree2_repo))
        git2r_error(__func__, NULL, "'tree1' and 'tree2' not from same repository", NULL);

    repository = git2r_repository_open(tree1_repo);
    if (!repository)
        git2r_error(__func__, NULL, git2r_err_invalid_repository, NULL);

    sha1 = git2r_get_list_element(tree1, "sha");
    error = git_revparse_single(&obj1, repository, CHAR(STRING_ELT(sha1, 0)));
    if (error)
	goto cleanup;

    sha2 = git2r_get_list_element(tree2, "sha");
    error = git_revparse_single(&obj2, repository, CHAR(STRING_ELT(sha2, 0)));
    if (error)
	goto cleanup;

    error = git_tree_lookup(&c_tree1, repository, git_object_id(obj1));
    if (error)
	goto cleanup;

    error = git_tree_lookup(&c_tree2, repository, git_object_id(obj2));
    if (error)
	goto cleanup;

    error = git_diff_tree_to_tree(
        &diff,
        repository,
        c_tree1,
        c_tree2,
        opts);
    if (error)
	goto cleanup;

    if (Rf_isNull(filename)) {
        PROTECT(result = Rf_mkNamed(VECSXP, git2r_S3_items__git_diff));
        nprotect++;
        Rf_setAttrib(result, R_ClassSymbol,
                     Rf_mkString(git2r_S3_class__git_diff));

        SET_VECTOR_ELT(
            result,
            git2r_S3_item__git_diff__old,
            tree1);

        SET_VECTOR_ELT(
            result,
            git2r_S3_item__git_diff__new,
            tree2);

        error = git2r_diff_format_to_r(diff, result);
    } else if (0 == Rf_length(filename)) {
        git_buf buf = GIT_BUF_INIT_CONST(NULL, 0);

        error = git_diff_to_buf(
            &buf,
            diff,
            GIT_DIFF_FORMAT_PATCH);
        if (!error) {
            PROTECT(result = Rf_mkString(buf.ptr));
            nprotect++;
        }

        GIT2R_BUF_DISPOSE(&buf);
    } else {
        FILE *fp = fopen(CHAR(STRING_ELT(filename, 0)), "w+");

        error = git_diff_print(
            diff,
            GIT_DIFF_FORMAT_PATCH,
            git_diff_print_callback__to_file_handle,
            fp);

        if (fp)
            fclose(fp);
    }

cleanup:
    free(opts->pathspec.strings);
    git_diff_free(diff);
    git_tree_free(c_tree1);
    git_tree_free(c_tree2);
    git_object_free(obj1);
    git_object_free(obj2);
    git_repository_free(repository);

    if (nprotect)
        UNPROTECT(nprotect);

    if (error)
	git2r_error(__func__, GIT2R_ERROR_LAST(), NULL, NULL);

    return result;
}

/**
 * Data structure to hold the information when counting diff objects.
 */
typedef struct {
    size_t num_files;
    size_t max_hunks;
    size_t max_lines;
    size_t num_hunks;
    size_t num_lines;
} git2r_diff_count_payload;

/**
 * Callback per file in the diff
 *
 * @param delta A pointer to the delta data for the file
 * @param progress Goes from 0 to 1 over the diff
 * @param payload A pointer to the git2r_diff_count_payload data structure
 * @return 0
 */
static int
git2r_diff_count_file_cb(
    const git_diff_delta *delta,
    float progress,
    void *payload)
{
    git2r_diff_count_payload *n = payload;

    GIT2R_UNUSED(delta);
    GIT2R_UNUSED(progress);

    n->num_files += 1;
    n->num_hunks = n->num_lines = 0;
    return 0;
}

/**
 * Callback per hunk in the diff
 *
 * @param delta A pointer to the delta data for the file
 * @param hunk A pointer to the structure describing a hunk of a diff
 * @param payload A pointer to the git2r_diff_count_payload data structure
 * @return 0
 */
static int
git2r_diff_count_hunk_cb(
    const git_diff_delta *delta,
    const git_diff_hunk *hunk,
    void *payload)
{
    git2r_diff_count_payload *n = payload;

    GIT2R_UNUSED(delta);
    GIT2R_UNUSED(hunk);

    n->num_hunks += 1;
    if (n->num_hunks > n->max_hunks)
        n->max_hunks = n->num_hunks;
    n->num_lines = 0;
    return 0;
}

/**
 * Callback per text diff line
 *
 * @param delta A pointer to the delta data for the file
 * @param hunk A pointer to the structure describing a hunk of a diff
 * @param line A pointer to the structure describing a line (or data
 * span) of a diff.
 * @param payload A pointer to the git2r_diff_count_payload data structure
 * @return 0
 */
static int
git2r_diff_count_line_cb(
    const git_diff_delta *delta,
    const git_diff_hunk *hunk,
    const git_diff_line *line,
    void *payload)
{
    git2r_diff_count_payload *n = payload;

    GIT2R_UNUSED(delta);
    GIT2R_UNUSED(hunk);
    GIT2R_UNUSED(line);

    n->num_lines += 1;
    if (n->num_lines > n->max_lines)
        n->max_lines = n->num_lines;
    return 0;
}


/**
 *  Count diff objects
 *
 * @param diff Pointer to the diff
 * @param num_files Pointer where to save the number of files
 * @param max_hunks Pointer where to save the maximum number of hunks
 * @param max_lines Pointer where to save the maximum number of lines
 * @return 0 if OK, else -1
 */
static int
git2r_diff_count(
    git_diff *diff,
    size_t *num_files,
    size_t *max_hunks,
    size_t *max_lines)
{
    int error;
    git2r_diff_count_payload n = { 0, 0, 0 };

    error = git_diff_foreach(
        diff,
        git2r_diff_count_file_cb,
        /* binary_cb */ NULL,
        git2r_diff_count_hunk_cb,
        git2r_diff_count_line_cb,
        /* payload= */ (void*) &n);

    if (error)
	return -1;

    *num_files = n.num_files;
    *max_hunks = n.max_hunks;
    *max_lines = n.max_lines;

    return 0;
}

/**
 * Data structure to hold the callback information when generating
 * diff objects.
 */
typedef struct {
    SEXP result;
    SEXP hunk_tmp;
    SEXP line_tmp;
    size_t file_ptr, hunk_ptr, line_ptr;
} git2r_diff_payload;

static int
git2r_diff_get_hunk_cb(
    const git_diff_delta *delta,
    const git_diff_hunk *hunk,
    void *payload);

static int
git2r_diff_get_line_cb(
    const git_diff_delta *delta,
    const git_diff_hunk *hunk,
    const git_diff_line *line,
    void *payload);

/**
 * Callback per file in the diff
 *
 * @param delta A pointer to the delta data for the file
 * @param progress Goes from 0 to 1 over the diff
 * @param payload A pointer to the git2r_diff_payload data structure
 * @return 0
 */
static int
git2r_diff_get_file_cb(
    const git_diff_delta *delta,
    float progress,
    void *payload)
{
    git2r_diff_payload *p = (git2r_diff_payload *) payload;

    GIT2R_UNUSED(progress);

    /* Save previous hunk's lines in hunk_tmp, we just call the
       hunk callback, with a NULL hunk */
    git2r_diff_get_hunk_cb(delta, /* hunk= */ 0, payload);

    /* Save the previous file's hunks from the hunk_tmp
       temporary storage. */
    if (p->file_ptr != 0) {
        SEXP hunks;
	size_t len=p->hunk_ptr, i;

	SET_VECTOR_ELT(
            VECTOR_ELT(p->result, p->file_ptr - 1),
            git2r_S3_item__git_diff_file__hunks,
            hunks = Rf_allocVector(VECSXP, p->hunk_ptr));
	for (i = 0; i < len ; i++)
	    SET_VECTOR_ELT(hunks, i, VECTOR_ELT(p->hunk_tmp, i));
    }

    /* OK, ready for next file, if any */
    if (delta) {
	SEXP file_obj;

        PROTECT(file_obj = Rf_mkNamed(VECSXP, git2r_S3_items__git_diff_file));
        Rf_setAttrib(
            file_obj,
            R_ClassSymbol,
            Rf_mkString(git2r_S3_class__git_diff_file));
        SET_VECTOR_ELT(p->result, p->file_ptr, file_obj);

	SET_VECTOR_ELT(
            file_obj,
            git2r_S3_item__git_diff_file__old_file,
            Rf_mkString(delta->old_file.path));

	SET_VECTOR_ELT(
            file_obj,
            git2r_S3_item__git_diff_file__new_file,
            Rf_mkString(delta->new_file.path));

	p->file_ptr++;
	p->hunk_ptr = 0;
	p->line_ptr = 0;
        UNPROTECT(1);
    }

    return 0;
}

/**
 * Process a hunk
 *
 * First we save the previous hunk, if there was one. Then create an
 * empty hunk (i.e. without any lines) and put it in the result.
 *
 * @param delta A pointer to the delta data for the file
 * @param hunk A pointer to the structure describing a hunk of a diff
 * @param payload Pointer to a git2r_diff_payload data structure
 * @return 0
 */
static int
git2r_diff_get_hunk_cb(
    const git_diff_delta *delta,
    const git_diff_hunk *hunk,
    void *payload)
{
    git2r_diff_payload *p = (git2r_diff_payload *) payload;

    GIT2R_UNUSED(delta);

    /* Save previous hunk's lines in hunk_tmp, from the line_tmp
       temporary storage. */
    if (p->hunk_ptr != 0) {
	SEXP lines;
	size_t len=p->line_ptr, i;

	SET_VECTOR_ELT(
            VECTOR_ELT(p->hunk_tmp, p->hunk_ptr-1),
            git2r_S3_item__git_diff_hunk__lines,
            lines = Rf_allocVector(VECSXP, p->line_ptr));
	for (i = 0; i < len; i++)
	    SET_VECTOR_ELT(lines, i, VECTOR_ELT(p->line_tmp, i));
    }

    /* OK, ready for the next hunk, if any */
    if (hunk) {
	SEXP hunk_obj;

        PROTECT(hunk_obj = Rf_mkNamed(VECSXP, git2r_S3_items__git_diff_hunk));
        Rf_setAttrib(
            hunk_obj,
            R_ClassSymbol,
            Rf_mkString(git2r_S3_class__git_diff_hunk));

        SET_VECTOR_ELT(
            hunk_obj,
            git2r_S3_item__git_diff_hunk__old_start,
            Rf_ScalarInteger(hunk->old_start));

	SET_VECTOR_ELT(
            hunk_obj,
            git2r_S3_item__git_diff_hunk__old_lines,
            Rf_ScalarInteger(hunk->old_lines));

        SET_VECTOR_ELT(
            hunk_obj,
            git2r_S3_item__git_diff_hunk__new_start,
            Rf_ScalarInteger(hunk->new_start));

	SET_VECTOR_ELT(
            hunk_obj,
            git2r_S3_item__git_diff_hunk__new_lines,
            Rf_ScalarInteger(hunk->new_lines));

	SET_VECTOR_ELT(
            hunk_obj,
            git2r_S3_item__git_diff_hunk__header,
            Rf_mkString(hunk->header));

	SET_VECTOR_ELT(p->hunk_tmp, p->hunk_ptr, hunk_obj);
        UNPROTECT(1);
	p->hunk_ptr += 1;
	p->line_ptr = 0;
    }

    return 0;
}

/**
 * Process a line
 *
 * This is easy, just populate a git_diff_line object and
 * put it in the temporary hunk.
 *
 * @param delta A pointer to the delta data for the file
 * @param hunk A pointer to the structure describing a hunk of a diff
 * @param line A pointer to the structure describing a line (or data
 * span) of a diff.
 * @param payload Pointer to a git2r_diff_payload data structure
 * @return 0
 */
static int
git2r_diff_get_line_cb(
    const git_diff_delta *delta,
    const git_diff_hunk *hunk,
    const git_diff_line *line,
    void *payload)
{
    git2r_diff_payload *p = (git2r_diff_payload *) payload;
    static char short_buffer[200];
    char *buffer = short_buffer;
    SEXP line_obj;

    GIT2R_UNUSED(delta);
    GIT2R_UNUSED(hunk);

    PROTECT(line_obj = Rf_mkNamed(VECSXP, git2r_S3_items__git_diff_line));
    Rf_setAttrib(
        line_obj,
        R_ClassSymbol,
        Rf_mkString(git2r_S3_class__git_diff_line));

    SET_VECTOR_ELT(
        line_obj,
        git2r_S3_item__git_diff_line__origin,
        Rf_ScalarInteger(line->origin));

    SET_VECTOR_ELT(
        line_obj,
        git2r_S3_item__git_diff_line__old_lineno,
        Rf_ScalarInteger(line->old_lineno));

    SET_VECTOR_ELT(
        line_obj,
        git2r_S3_item__git_diff_line__new_lineno,
        Rf_ScalarInteger(line->new_lineno));

    SET_VECTOR_ELT(
        line_obj,
        git2r_S3_item__git_diff_line__num_lines,
        Rf_ScalarInteger(line->num_lines));

    if (line->content_len > sizeof(buffer))
	buffer = malloc(line->content_len+1);
    memcpy(buffer, line->content, line->content_len);
    buffer[line->content_len] = 0;

    SET_VECTOR_ELT(
        line_obj,
        git2r_S3_item__git_diff_line__content,
        Rf_mkString(buffer));

    if (buffer != short_buffer)
	free(buffer);

    SET_VECTOR_ELT(p->line_tmp, p->line_ptr++, line_obj);
    UNPROTECT(1);

    return 0;
}

/**
 * Format a diff as an R object
 *
 * libgit2 has callbacks to walk over the files, hunks and line
 * of a diff. This means that we need to walk over the diff twice,
 * if we don't want to reallocate our lists over and over again (or write a
 * smart list that preallocates memory).
 *
 * So we walk over it first and calculate the maximum number of
 * hunks in a file, and the maximum number of lines in a hunk.
 *
 * Then in the second walk, we have a correspondingly allocated
 * list that we use for temporary storage.
 *
 * @param diff Pointer to the diff
 * @param dest The S3 class git_diff to hold the formated diff
 * @return 0 if OK, else error code
 */
static int
git2r_diff_format_to_r(
    git_diff *diff,
    SEXP dest)
{
    int error, nprotect = 0;
    git2r_diff_payload payload = { /* result=   */ R_NilValue,
                                   /* hunk_tmp= */ R_NilValue,
                                   /* line_tmp= */ R_NilValue,
                                   /* file_ptr= */ 0,
                                   /* hunk_ptr= */ 0,
                                   /* line_ptr= */ 0 };

    size_t num_files, max_hunks, max_lines;

    error = git2r_diff_count(diff, &num_files, &max_hunks, &max_lines);

    if (error)
        return error;

    SET_VECTOR_ELT(
        dest,
        git2r_S3_item__git_diff__files,
        payload.result = Rf_allocVector(VECSXP, num_files));
    PROTECT(payload.hunk_tmp = Rf_allocVector(VECSXP, max_hunks));
    nprotect++;
    PROTECT(payload.line_tmp = Rf_allocVector(VECSXP, max_lines));
    nprotect++;

    error = git_diff_foreach(
        diff,
        git2r_diff_get_file_cb,
        /* binary_cb */ NULL,
        git2r_diff_get_hunk_cb,
        git2r_diff_get_line_cb,
        &payload);
    if (!error) {
        /* Need to call them once more, to put in the last lines/hunks/files. */
        error = git2r_diff_get_file_cb(
            /* delta=    */ NULL,
            /* progress= */ 100,
            &payload);
    }

    if (nprotect)
        UNPROTECT(nprotect);

    return error;
}
