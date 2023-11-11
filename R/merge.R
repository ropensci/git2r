## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2023 The git2r contributors
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License, version 2,
## as published by the Free Software Foundation.
##
## git2r is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program; if not, write to the Free Software Foundation, Inc.,
## 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

##' Find a merge base between two commits
##'
##' @param one One of the commits
##' @param two The other commit
##' @return git_commit
##' @export
##' @examples \dontrun{
##' ## Create a directory in tempdir
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##'
##' ## Initialize a repository
##' repo <- init(path)
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Create a file, add and commit
##' writeLines("Master branch", file.path(path, "master_branch.txt"))
##' add(repo, "master_branch.txt")
##' commit_1 <- commit(repo, "Commit message 1")
##'
##' ## Create first branch, checkout, add file and commit
##' branch_1 <- branch_create(commit_1, "branch_1")
##' checkout(branch_1)
##' writeLines("Branch 1", file.path(path, "branch_1.txt"))
##' add(repo, "branch_1.txt")
##' commit_2 <- commit(repo, "Commit message branch_1")
##'
##' ## Create second branch, checkout, add file and commit
##' branch_2 <- branch_create(commit_1, "branch_2")
##' checkout(branch_2)
##' writeLines("Branch 2", file.path(path, "branch_2.txt"))
##' add(repo, "branch_2.txt")
##' commit_3 <- commit(repo, "Commit message branch_2")
##'
##' ## Check that merge base equals commit_1
##' stopifnot(identical(merge_base(commit_2, commit_3), commit_1))
##' }
merge_base <- function(one = NULL, two = NULL) {
    .Call(git2r_merge_base, one, two)
}

##' Merge a branch into HEAD
##'
##' @rdname merge
##' @param x A path (default '.') to a repository, or a
##'     \code{git_repository} object, or a \code{git_branch}.
##' @param y If \code{x} is a \code{git_repository}, the name of the
##'     branch to merge into HEAD. Not used if \code{x} is a
##'     \code{git_branch}.
##' @param commit_on_success If there are no conflicts written to the
##'     index, the merge commit will be committed. Default is TRUE.
##' @param merger Who made the merge. The default (\code{NULL}) is to
##'     use \code{default_signature} for the repository.
##' @param fail If a conflict occurs, exit immediately instead of
##'     attempting to continue resolving conflicts. Default is
##'     \code{FALSE}.
##' @param ... Additional arguments (unused).
##' @template return-git_merge_result
##' @export
##' @template merge-example
merge.git_branch <- function(x, y = NULL, commit_on_success = TRUE,
                             merger = NULL, fail = FALSE, ...) {
    if (is.null(merger))
        merger <- default_signature(x$repo)
    .Call(git2r_merge_branch, x, merger, commit_on_success, fail)
}

##' @export
##' @rdname merge
merge.git_repository <- function(x, y = NULL, commit_on_success = TRUE,
                                 merger = NULL, fail = FALSE, ...) {
    ## Check branch argument
    if (!is.character(y) || !identical(length(y), 1L))
        stop("'branch' must be a character vector of length one")
    b <- branches(x)
    b <- b[vapply(b, "[[", character(1), "name") == y][[1]]
    merge.git_branch(b, commit_on_success = commit_on_success,
                     merger = merger, fail = fail)
}

##' @export
##' @rdname merge
merge.character <- function(x = ".", y = NULL, commit_on_success = TRUE,
                            merger = NULL, fail = FALSE, ...) {
    x <- lookup_repository(x)
    merge.git_repository(x, y, commit_on_success, merger, fail)
}

##' @export
base::merge

##' @export
format.git_merge_result <- function(x, ...) {
    if (isTRUE(x$up_to_date))
        return("Already up-to-date")
    if (isTRUE(x$conflicts))
        return("Merge: Conflicts")
    if (isTRUE(x$fast_forward))
        return("Merge: Fast-forward")
    return("Merge")
}

##' @export
print.git_merge_result <- function(x, ...) {
    cat(format(x, ...), "\n", sep = "")
    invisible(x)
}
