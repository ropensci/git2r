/*
 *  git2r, R bindings to the libgit2 library.
 *  Copyright (C) 2013-2018 The git2r contributors
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

#include <string.h>

#include "git2r_objects.h"

const char *git2r_S3_class__git_blame = "git_blame";
const char *git2r_S3_items__git_blame[] = {
    "path", "hunks", "repo", ""};

const char *git2r_S3_class__git_blame_hunk;
const char *git2r_S3_items__git_blame_hunk[] = {
    "lines_in_hunk", "final_commit_id", "final_start_line_number",
    "final_signature", "orig_commit_id", "orig_start_line_number",
    "orig_signature", "orig_path", "boundary", "repo", ""};

const char *git2r_S3_class__git_blob = "git_blob";
const char *git2r_S3_items__git_blob[] = {
    "sha", "repo", ""};

const char *git2r_S3_class__git_branch = "git_branch";
const char *git2r_S3_items__git_branch[] = {
    "name", "type", "repo", ""};

const char *git2r_S3_class__git_commit = "git_commit";
const char *git2r_S3_items__git_commit[] = {
    "sha", "author", "committer", "summary", "message", "repo", ""};

const char *git2r_S3_class__git_merge_result = "git_merge_result";
const char *git2r_S3_items__git_merge_result[] = {
    "up_to_date", "fast_forward", "conflicts", "sha", ""};

const char *git2r_S3_class__git_repository = "git_repository";
const char *git2r_S3_items__git_repository[] = {
    "path", ""};

const char *git2r_S3_class__git_signature = "git_signature";
const char *git2r_S3_items__git_signature[] = {
    "name", "email", "when", ""};

const char *git2r_S3_class__git_tag = "git_tag";
const char *git2r_S3_items__git_tag[] = {
    "sha", "message", "name", "tagger", "target", "repo", ""};

const char *git2r_S3_class__git_time = "git_time";
const char *git2r_S3_items__git_time[] = {
    "time", "offset", ""};

const char *git2r_S3_class__git_transfer_progress = "git_transfer_progress";
const char *git2r_S3_items__git_transfer_progress[] = {
    "total_objects", "indexed_objects", "received_objects",
    "local_objects", "total_deltas", "indexed_deltas",
    "received_bytes", ""};

const char *git2r_S3_class__git_tree = "git_tree";
const char *git2r_S3_items__git_tree[] = {
    "sha", "filemode", "type", "id", "name", "repo", ""};

/**
 * Get the list element named str, or return NULL.
 *
 * From the manual 'Writing R Extensions'
 * (https://cran.r-project.org/doc/manuals/r-release/R-exts.html)
 */
SEXP git2r_get_list_element(SEXP list, const char *str)
{
    int i = 0;
    SEXP elmt = R_NilValue, names = Rf_getAttrib(list, R_NamesSymbol);

    for (; i < Rf_length(list); i++) {
        if(strcmp(CHAR(STRING_ELT(names, i)), str) == 0) {
            elmt = VECTOR_ELT(list, i);
            break;
        }
    }

    return elmt;
}
