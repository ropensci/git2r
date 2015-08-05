## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2015 The git2r contributors
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
##' @rdname reset-methods
##' @docType methods
##' @param commit The \code{\linkS4class{git_commit}} to which the
##' HEAD should be moved to.
##' @param reset_type Kind of reset operation to perform. 'soft' means
##' the Head will be moved to the commit. 'mixed' reset will trigger a
##' 'soft' reset, plus the index will be replaced with the content of
##' the commit tree. 'hard' reset will trigger a 'mixed' reset and the
##' working directory will be replaced with the content of the index.
##' @return invisible NULL
##' @keywords methods
##' @include S4_classes.r
##' @examples \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' # Configure a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Create a file, add and commit
##' writeLines("Hello world!", file.path(path, "test-1.txt"))
##' add(repo, 'test-1.txt')
##' commit_1 <- commit(repo, "Commit message")
##'
##' ## Make one more commit
##' writeLines(c("Hello world!", "HELLO WORLD!"), file.path(path, "test-1.txt"))
##' add(repo, 'test-1.txt')
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
##' add(repo, 'test-1.txt')
##' commit(repo, "Next commit message")
##' reset(commit_1, "hard")
##' status(repo)
##' }
setGeneric("reset",
           signature = c("commit"),
           function(commit, reset_type = c("soft", "mixed", "hard"))
           standardGeneric("reset"))

##' @rdname reset-methods
##' @export
setMethod("reset",
          signature(commit = "git_commit"),
          function(commit, reset_type)
          {
              reset_type <- switch(match.arg(reset_type),
                                   soft  = 1L,
                                   mixed = 2L,
                                   hard  = 3L)

              invisible(.Call(git2r_reset, commit, reset_type))
          }
)
