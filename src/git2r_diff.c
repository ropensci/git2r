/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2014 The git2r contributor
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

#include "git2r_diff.h"
#include "git2r_error.h"
#include "git2r_tree.h"
#include "git2r_repository.h"

#include "git2.h"

#include <stdlib.h>
#include <string.h>

int git2r_diff_count(git_diff *diff, size_t *num_files,
		     size_t *max_hunks, size_t *max_lines);
SEXP git2r_diff_format_to_r(git_diff *diff, SEXP old, SEXP new);
SEXP git2r_diff_index_to_wd(SEXP repo);
SEXP git2r_diff_head_to_index(SEXP repo);
SEXP git2r_diff_tree_to_wd(SEXP tree);
SEXP git2r_diff_tree_to_index(SEXP tree);
SEXP git2r_diff_tree_to_tree(SEXP tree1, SEXP tree2);

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
 * @param tree2 The second gree to compare.
 * @param index Whether to compare to the index.
 * @return The diff.
 */

SEXP git2r_diff(SEXP repo, SEXP tree1, SEXP tree2, SEXP index)
{
    int c_index=LOGICAL(index)[0];

    if (isNull(tree1) && ! c_index) {
	if (!isNull(tree2))
	    Rf_error("Invalid diff parameters");
	return git2r_diff_index_to_wd(repo);
    } else if (isNull(tree1) && c_index) {
	if (!isNull(tree2))
	    Rf_error("Invalid diff parameters");
	return git2r_diff_head_to_index(repo);
    } else if (!isNull(tree1) && isNull(tree2) && ! c_index) {
	if (!isNull(repo))
	    Rf_error("Invalid diff parameters");
	return git2r_diff_tree_to_wd(tree1);
    } else if (!isNull(tree1) && isNull(tree2) && c_index) {
	if (!isNull(repo))
	    Rf_error("Invalid diff parameters");
	return git2r_diff_tree_to_index(tree1);
    } else {
	if (!isNull(repo))
	    Rf_error("Invalid diff parameters");
	return git2r_diff_tree_to_tree(tree1, tree2);
    }
}

/**
 * :TODO:DOCUMENTATION:
 *
 * @param repo :TODO:DOCUMENTATION:
 * @return :TODO:DOCUMENTATION:
 */

SEXP git2r_diff_index_to_wd(SEXP repo)
{
    int err;
    git_repository *repository = NULL;
    git_diff *diff = NULL;
    SEXP result = R_NilValue;

    repository = git2r_repository_open(repo);
    if (!repository)
        Rf_error(git2r_err_invalid_repository);

    err = git_diff_index_to_workdir(&diff,
				    repository,
				    /*index=*/ NULL,
				    /*opts=*/ NULL);

    if (err != 0)
	goto cleanup;

    result = git2r_diff_format_to_r(diff, /* old= */ mkString("index"),
				    /* new= */ mkString("workdir"));

cleanup:

    if (diff)
	git_diff_free(diff);

    if (repository)
        git_repository_free(repository);

    if (err != 0)
        Rf_error("Error: %s\n", giterr_last()->message);

    return result;
}

/**
 * :TODO:DOCUMENTATION:
 *
 * @param repo :TODO:DOCUMENTATION:
 * @return :TODO:DOCUMENTATION:
 */

SEXP git2r_diff_head_to_index(SEXP repo)
{
    int err;
    git_repository *repository = NULL;
    git_diff *diff = NULL;
    git_object *obj = NULL;
    git_tree *head = NULL;
    SEXP result = R_NilValue;

    repository = git2r_repository_open(repo);
    if (!repository)
        Rf_error(git2r_err_invalid_repository);

    err = git_revparse_single(&obj, repository, "HEAD^{tree}");

    if (err != 0)
	goto cleanup;

    err = git_tree_lookup(&head, repository, git_object_id(obj));

    if (err != 0)
	goto cleanup;

    err = git_diff_tree_to_index(&diff,
				 repository,
				 head,
				 /* index= */ NULL,
				 /* opts = */ NULL);

    if (err != 0)
	goto cleanup;

    /* TODO: object instead of HEAD string */
    result = git2r_diff_format_to_r(diff, /* old= */ mkString("HEAD"),
				    /* new=*/ mkString("index"));

cleanup:

    if (head)
	git_tree_free(head);

    if (obj)
	git_object_free(obj);

    if (diff)
	git_diff_free(diff);

    if (repository)
        git_repository_free(repository);

    if (err != 0)
        Rf_error("Error: %s\n", giterr_last()->message);

    return result;
}

/**
 * :TODO:DOCUMENTATION:
 *
 * @param repo :TODO:DOCUMENTATION:
 * @param tree :TODO:DOCUMENTATION:
 * @return :TODO:DOCUMENTATION:
 */

SEXP git2r_diff_tree_to_wd(SEXP tree)
{
    int err;
    git_repository *repository = NULL;
    git_diff *diff = NULL;
    git_object *obj = NULL;
    git_tree *c_tree = NULL;
    SEXP hex;
    SEXP result;
    SEXP repo = GET_SLOT(tree, Rf_install("repo"));

    repository = git2r_repository_open(repo);
    if (!repository)
        Rf_error(git2r_err_invalid_repository);

    hex = GET_SLOT(tree, Rf_install("hex"));
    err = git_revparse_single(&obj, repository,
			      CHAR(STRING_ELT(hex, 0)));

    if (err != 0)
	goto cleanup;

    err = git_tree_lookup(&c_tree, repository, git_object_id(obj));

    if (err != 0)
	goto cleanup;

    err = git_diff_tree_to_workdir(&diff,
				   repository,
				   c_tree,
				   /* opts = */ NULL);

    if (err != 0)
	goto cleanup;

    result = git2r_diff_format_to_r(diff, /* old= */ tree,
				    /* new= */ mkString("workdir"));

cleanup:
    if (diff)
        git_diff_free(diff);

    if (c_tree)
	git_tree_free(c_tree);

    if (obj)
	git_object_free(obj);

    if (repository)
	git_repository_free(repository);

    if (err != 0)
        Rf_error("Error: %s\n", giterr_last()->message);

    return result;
}

/**
 * :TODO:DOCUMENTATION:
 *
 * @param repo :TODO:DOCUMENTATION:
 * @param tree :TODO:DOCUMENTATION:
 * @return :TODO:DOCUMENTATION:
 */

SEXP git2r_diff_tree_to_index(SEXP tree)
{
    int err;
    git_repository *repository = NULL;
    git_diff *diff = NULL;
    git_object *obj = NULL;
    git_tree *c_tree = NULL;
    SEXP hex;
    SEXP result;
    SEXP repo = GET_SLOT(tree, Rf_install("repo"));

    repository = git2r_repository_open(repo);
    if (!repository)
        Rf_error(git2r_err_invalid_repository);

    hex = GET_SLOT(tree, Rf_install("hex"));
    err = git_revparse_single(&obj, repository,
			      CHAR(STRING_ELT(hex, 0)));

    if (err != 0)
	goto cleanup;

    err = git_tree_lookup(&c_tree, repository, git_object_id(obj));

    if (err != 0)
	goto cleanup;

    err = git_diff_tree_to_index(&diff,
				 repository,
				 c_tree,
				 /* index= */ NULL,
				 /* opts= */ NULL);

    if (err != 0)
	goto cleanup;

    result = git2r_diff_format_to_r(diff, /* old= */ tree,
				    /* new= */ mkString("index"));

cleanup:
    if (diff)
        git_diff_free(diff);

    if (c_tree)
	git_tree_free(c_tree);

    if (obj)
	git_object_free(obj);

    if (repository)
	git_repository_free(repository);

    if (err != 0)
	Rf_error("Error: %s\n", giterr_last()->message);

    return result;

}

/**
 * :TODO:DOCUMENTATION:
 *
 * @param tree1 :TODO:DOCUMENTATION:
 * @param tree2 :TODO:DOCUMENTATION:
 * @return :TODO:DOCUMENTATION:
 */

SEXP git2r_diff_tree_to_tree(SEXP tree1, SEXP tree2)
{
    int err;
    git_repository *repository = NULL;
    git_diff *diff = NULL;
    git_object *obj1 = NULL, *obj2 = NULL;
    git_tree *c_tree1 = NULL, *c_tree2 = NULL;
    SEXP result;
    SEXP repo = GET_SLOT(tree1, Rf_install("repo"));
    SEXP hex1, hex2;

    /* We already checked that tree2 is from the same repo, in R */
    repository = git2r_repository_open(repo);
    if (!repository)
        Rf_error(git2r_err_invalid_repository);

    hex1 = GET_SLOT(tree1, Rf_install("hex"));
    err = git_revparse_single(&obj1, repository,
			      CHAR(STRING_ELT(hex1, 0)));

    if (err != 0)
	goto cleanup;

    hex2 = GET_SLOT(tree2, Rf_install("hex"));
    err = git_revparse_single(&obj2, repository,
			      CHAR(STRING_ELT(hex2, 0)));

    if (err != 0)
	goto cleanup;

    err = git_tree_lookup(&c_tree1, repository, git_object_id(obj1));

    if (err != 0)
	goto cleanup;

    err = git_tree_lookup(&c_tree2, repository, git_object_id(obj2));

    if (err != 0)
	goto cleanup;

    err = git_diff_tree_to_tree(&diff,
				repository,
				c_tree1,
				c_tree2,
				/* opts= */ NULL);

    if (err != 0)
	goto cleanup;

    result = git2r_diff_format_to_r(diff, /* old= */ tree1,
				    /* new= */ tree2);

cleanup:
    if (diff)
        git_diff_free(diff);

    if (c_tree1)
	git_tree_free(c_tree1);

    if (c_tree2)
	git_tree_free(c_tree2);

    if (obj1)
	git_object_free(obj1);

    if (obj2)
	git_object_free(obj2);

    if (repository)
	git_repository_free(repository);

    if (err != 0)
	Rf_error("Error: %s\n", giterr_last()->message);

    return result;
}

/**
 * :TODO:DOCUMENTATION:
 */
typedef struct {
    size_t num_files;
    size_t max_hunks;
    size_t max_lines;
    size_t num_hunks;
    size_t num_lines;
} git2r_diff_count_payload;

/**
 * :TODO:DOCUMENTATION:
 *
 * @param delta :TODO:DOCUMENTATION:
 * @param progre :TODO:DOCUMENTATION:
 * @param payload :TODO:DOCUMENTATION:
 * @return :TODO:DOCUMENTATION:
 */

int git2r_diff_count_file_cb(const git_diff_delta *delta,
			     float progress,
			     void *payload)
{
    git2r_diff_count_payload *n = payload;
    n->num_files += 1;
    n->num_hunks = n->num_lines = 0;
    return 0;
}

/**
 * :TODO:DOCUMENTATION:
 *
 * @param delta :TODO:DOCUMENTATION:
 * @param hunk :TODO:DOCUMENTATION:
 * @param payload :TODO:DOCUMENTATION:
 * @return :TODO:DOCUMENTATION:
 */

int git2r_diff_count_hunk_cb(const git_diff_delta *delta,
			     const git_diff_hunk *hunk,
			     void *payload)
{
    git2r_diff_count_payload *n = payload;
    n->num_hunks += 1;
    if (n->num_hunks > n->max_hunks) { n->max_hunks = n->num_hunks; }
    n->num_lines = 0;
    return 0;
}

/**
 * :TODO:DOCUMENTATION:
 *
 * @param delta :TODO:DOCUMENTATION:
 * @param hunk :TODO:DOCUMENTATION:
 * @param line  :TODO:DOCUMENTATION:
 * @param payload :TODO:DOCUMENTATION:
 * @return :TODO:DOCUMENTATION:
 */

int git2r_diff_count_line_cb(const git_diff_delta *delta,
			     const git_diff_hunk *hunk,
			     const git_diff_line *line,
			     void *payload)
{
    git2r_diff_count_payload *n = payload;
    n->num_lines += 1;
    if (n->num_lines > n->max_lines) { n->max_lines = n->num_lines; }
    return 0;
}


/**
 * :TODO:DOCUMENTATION:
 *
 * @param diff :TODO:DOCUMENTATION:
 * @param num_file :TODO:DOCUMENTATION:
 * @param num_hunk :TODO:DOCUMENTATION:
 * @return :TODO:DOCUMENTATION:
 */

int git2r_diff_count(git_diff *diff, size_t *num_files,
		     size_t *max_hunks, size_t *max_lines)
{

    int err;
    git2r_diff_count_payload n = { 0, 0, 0 };

    err = git_diff_foreach(diff,
			   git2r_diff_count_file_cb,
			   git2r_diff_count_hunk_cb,
			   git2r_diff_count_line_cb,
			   /* payload= */ (void*) &n);

    if (err != 0)
	return -1;

    *num_files = n.num_files;
    *max_hunks = n.max_hunks;
    *max_lines = n.max_lines;

    return 0;
}

/**
 * :TODO:DOCUMENTATION:
 */
typedef struct {
    SEXP result;
    SEXP hunk_tmp;
    SEXP line_tmp;
    size_t file_ptr, hunk_ptr, line_ptr;
} git2r_diff_payload;

int git2r_diff_get_hunk_cb(const git_diff_delta *delta,
			   const git_diff_hunk *hunk,
			   void *payload);

int git2r_diff_get_line_cb(const git_diff_delta *delta,
			   const git_diff_hunk *hunk,
			   const git_diff_line *line,
			   void *payload);

/**
 * :TODO:DOCUMENTATION:
 *
 * @param delta :TODO:DOCUMENTATION:
 * @param progre :TODO:DOCUMENTATION:
 * @param payload :TODO:DOCUMENTATION:
 * @return :TODO:DOCUMENTATION:
 */

int git2r_diff_get_file_cb(const git_diff_delta *delta,
			   float progress,
			   void *payload)
{

    git2r_diff_payload *p = (git2r_diff_payload *) payload;

    /* Save previous hunk's lines in hunk_tmp, we just call the
       hunk callback, with a NULL hunk */
    git2r_diff_get_hunk_cb(delta, /* hunk= */ 0, payload);

    /* Save the previous file's hunks from the hunk_tmp
       temporary storage. */
    if (p->file_ptr != 0) {
	SEXP hunks =  allocVector(VECSXP, p->hunk_ptr);
	size_t len=p->hunk_ptr, i;
	for (i = 0; i < len ; i++) {
	    SET_VECTOR_ELT(hunks, i, VECTOR_ELT(p->hunk_tmp, i));
	}
	SET_SLOT(VECTOR_ELT(p->result, p->file_ptr-1), Rf_install("hunks"),
		 hunks);
    }

    /* OK, ready for next file, if any */
    if (delta) {
	SEXP file_obj;

	PROTECT(file_obj = NEW_OBJECT(MAKE_CLASS("git_diff_file")));
	SET_SLOT(file_obj, Rf_install("old_file"),
		 mkString(delta->old_file.path));
	SET_SLOT(file_obj, Rf_install("new_file"),
		 mkString(delta->new_file.path));
	SET_VECTOR_ELT(p->result, p->file_ptr, file_obj);

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
 * @param delta :TODO:DOCUMENTATION:
 * @param hunk :TODO:DOCUMENTATION:
 * @param payload :TODO:DOCUMENTATION:
 * @return :TODO:DOCUMENTATION:
 */

int git2r_diff_get_hunk_cb(const git_diff_delta *delta,
			   const git_diff_hunk *hunk,
			   void *payload)
{
    git2r_diff_payload *p = (git2r_diff_payload *) payload;

    /* Save previous hunk's lines in hunk_tmp, from the line_tmp
       temporary storage. */
    if (p->hunk_ptr != 0) {
	SEXP lines = allocVector(VECSXP, p->line_ptr);
	size_t len=p->line_ptr, i;
	for (i = 0; i < len; i++) {
	    SET_VECTOR_ELT(lines, i, VECTOR_ELT(p->line_tmp, i));
	}
	SET_SLOT(VECTOR_ELT(p->hunk_tmp, p->hunk_ptr-1),
		 Rf_install("lines"), lines);
    }

    /* OK, ready for the next hunk, if any */
    if (hunk) {
	SEXP hunk_obj;

	PROTECT(hunk_obj = NEW_OBJECT(MAKE_CLASS("git_diff_hunk")));
	SET_SLOT(hunk_obj, Rf_install("old_start"),
		 ScalarInteger(hunk->old_start));
	SET_SLOT(hunk_obj, Rf_install("old_lines"),
		 ScalarInteger(hunk->old_lines));
	SET_SLOT(hunk_obj, Rf_install("new_start"),
		 ScalarInteger(hunk->new_start));
	SET_SLOT(hunk_obj, Rf_install("new_lines"),
		 ScalarInteger(hunk->new_lines));
	SET_SLOT(hunk_obj, Rf_install("header"), mkString(hunk->header));
	SET_VECTOR_ELT(p->hunk_tmp, p->hunk_ptr, hunk_obj);

	p->hunk_ptr += 1;
	p->line_ptr = 0;

	UNPROTECT(1);
    }

    return 0;
}

/**
 * Process a line
 *
 * This is easy, just populate a git_diff_line object and
 * put it in the temporary hunk.
 *
 * @param delta :TODO:DOCUMENTATION:
 * @param hunk :TODO:DOCUMENTATION:
 * @param line :TODO:DOCUMENTATION:
 * @param payload :TODO:DOCUMENTATION:
 * @return :TODO:DOCUMENTATION:
 */

int git2r_diff_get_line_cb(const git_diff_delta *delta,
			   const git_diff_hunk *hunk,
			   const git_diff_line *line,
			   void *payload)
{
    git2r_diff_payload *p = (git2r_diff_payload *) payload;
    static char short_buffer[200];
    char *buffer = short_buffer;
    SEXP line_obj;

    PROTECT(line_obj = NEW_OBJECT(MAKE_CLASS("git_diff_line")));
    SET_SLOT(line_obj, Rf_install("origin"), ScalarInteger(line->origin));
    SET_SLOT(line_obj, Rf_install("old_lineno"),
	     ScalarInteger(line->old_lineno));
    SET_SLOT(line_obj, Rf_install("new_lineno"),
	     ScalarInteger(line->new_lineno));
    SET_SLOT(line_obj, Rf_install("num_lines"),
	     ScalarInteger(line->num_lines));

    if (line->content_len > sizeof(buffer)) {
	buffer = malloc(line->content_len+1);
    }
    memcpy(buffer, line->content, line->content_len);
    buffer[line->content_len] = 0;

    SET_SLOT(line_obj, Rf_install("content"), mkString(buffer));

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
 * @param diff :TODO:DOCUMENTATION:
 * @return :TODO:DOCUMENTATION:
 */

SEXP git2r_diff_format_to_r(git_diff *diff, SEXP old, SEXP new)
{
  int err;
  git2r_diff_payload payload = { /* result=   */ R_NilValue,
				 /* hunk_tmp= */ R_NilValue,
				 /* line_tmp= */ R_NilValue,
				 /* file_ptr= */ 0,
				 /* hunk_ptr= */ 0,
				 /* line_ptr= */ 0 };

  size_t num_files, max_hunks, max_lines;
  SEXP result;

  err = git2r_diff_count(diff, &num_files, &max_hunks, &max_lines);

  if (err != 0)
      Rf_error("cannot diff, internal error");

  PROTECT(payload.result = allocVector(VECSXP, num_files));
  PROTECT(payload.hunk_tmp = allocVector(VECSXP, max_hunks));
  PROTECT(payload.line_tmp = allocVector(VECSXP, max_lines));

  err = git_diff_foreach(diff,
			 git2r_diff_get_file_cb,
			 git2r_diff_get_hunk_cb,
			 git2r_diff_get_line_cb,
			 &payload);
  if (err != 0) {
      UNPROTECT(3);
      Rf_error("Error: %s\n", giterr_last()->message);
  }

  /* Need to call them once more, to put in the last lines/hunks/files. */
  git2r_diff_get_file_cb(/* delta= */ NULL, /* progress= */ 100, &payload);

  PROTECT(result = NEW_OBJECT(MAKE_CLASS("git_diff")));
  SET_SLOT(result, Rf_install("old"), old);
  SET_SLOT(result, Rf_install("new"), new);
  SET_SLOT(result, Rf_install("files"), payload.result);

  UNPROTECT(4);
  return result;
}
