## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2019 The git2r contributors
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

##' Reset current HEAD to the specified state
##'
##' @param object Either a \code{git_commit}, a \code{git_repository}
##'     or a character vector. If \code{object} is a
##'     \code{git_commit}, HEAD is moved to the \code{git_commit}. If
##'     \code{object} is a \code{git_repository}, resets the index
##'     entries in the \code{path} argument to their state at HEAD. If
##'     \code{object} is a character vector with paths, resets the
##'     index entries in \code{object} to their state at HEAD if the
##'     current working directory is in a repository.
##' @param reset_type If object is a 'git_commit', the kind of reset
##'     operation to perform. 'soft' means the HEAD will be moved to
##'     the commit. 'mixed' reset will trigger a 'soft' reset, plus
##'     the index will be replaced with the content of the commit
##'     tree. 'hard' reset will trigger a 'mixed' reset and the
##'     working directory will be replaced with the content of the
##'     index.
##' @param path If object is a 'git_repository', resets the index
##'     entries for all paths to their state at HEAD.
##' @return invisible NULL
##' @export
##' @examples \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' # Configure a user
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Create a file, add and commit
##' writeLines("Hello world!", file.path(path, "test-1.txt"))
##' add(repo, "test-1.txt")
##' commit_1 <- commit(repo, "Commit message")
##'
##' ## Change and stage the file
##' writeLines(c("Hello world!", "HELLO WORLD!"), file.path(path, "test-1.txt"))
##' add(repo, "test-1.txt")
##' status(repo)
##'
##' ## Unstage file
##' reset(repo, path = "test-1.txt")
##' status(repo)
##'
##' ## Make one more commit
##' add(repo, "test-1.txt")
##' commit(repo, "Next commit message")
##'
##' ## Create one more file
##' writeLines("Hello world!", file.path(path, "test-2.txt"))
##'
##' ## 'soft' reset to first commit and check status
##' reset(commit_1)
##' status(repo)
##'
##' ## 'mixed' reset to first commit and check status
##' commit(repo, "Next commit message")
##' reset(commit_1, "mixed")
##' status(repo)
##'
##' ## 'hard' reset to first commit and check status
##' add(repo, "test-1.txt")
##' commit(repo, "Next commit message")
##' reset(commit_1, "hard")
##' status(repo)
##' }
reset <- function(object,
                  reset_type = c("soft", "mixed", "hard"),
                  path = NULL) {
    if (is_commit(object)) {
        reset_type <- switch(match.arg(reset_type),
                             soft  = 1L,
                             mixed = 2L,
                             hard  = 3L)

        .Call(git2r_reset, object, reset_type)
    } else {
        object <- lookup_repository(object)
        if (is_empty(object)) {
            .Call(git2r_index_remove_bypath, object, path)
        } else {
            .Call(git2r_reset_default, object, path)
        }
    }

    invisible(NULL)
}
