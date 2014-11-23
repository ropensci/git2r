## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2014 The git2r contributors
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
##' @rdname merge_base-methods
##' @docType methods
##' @param one One of the commits
##' @param two The other commit
##' @return S4 class git_commit
##' @keywords methods
##' @include S4_classes.r
##' @examples \dontrun{
##' ## Create a directory in tempdir
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##'
##' ## Initialize a repository
##' repo <- init(path)
##' config(repo, user.name="Repo", user.email="repo@@example.org")
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
setGeneric("merge_base",
           signature = c("one", "two"),
           function(one, two)
           standardGeneric("merge_base"))

##' @rdname merge_base-methods
##' @export
setMethod("merge_base",
          signature(one = "git_commit",
                    two = "git_commit"),
          function(one, two)
          {
              stopifnot(identical(one@repo, two@repo))
              .Call(git2r_merge_base, one, two)
          }
)

##' Merge a branch into HEAD
##'
##' @rdname merge-methods
##' @docType methods
##' @param object A \code{\linkS4class{git_branch}} or
##' \code{\linkS4class{git_repository}} object.
##' @param ... Additional arguments affecting the merge
##' @param branch Name of branch if \code{object} is a
##' \code{\linkS4class{git_repository}}
##' @param commit_on_success If there are no conflicts written to the
##' index, the merge commit will be committed. Default is TRUE.
##' @param merger Who made the merge.
##' @return A \code{\linkS4class{git_merge_result}} object.
##' @keywords methods
##' @include S4_classes.r
##' @examples \dontrun{
##' ## Create a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##' config(repo, user.name = "User", user.email = "user@@example.org")
##'
##' ## Create a file, add and commit
##' writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'            con = file.path(path, "test.txt"))
##' add(repo, "test.txt")
##' commit_1 <- commit(repo, "Commit message 1")
##'
##' ## Create first branch, checkout, add file and commit
##' checkout(repo, "branch1", create = TRUE)
##' writeLines("Branch 1", file.path(path, "branch-1.txt"))
##' add(repo, "branch-1.txt")
##' commit(repo, "Commit message branch 1")
##'
##' ## Create second branch, checkout, add file and commit
##' b_2 <- branch_create(commit_1, "branch2")
##' checkout(b_2)
##' writeLines("Branch 2", file.path(path, "branch-2.txt"))
##' add(repo, "branch-2.txt")
##' commit(repo, "Commit message branch 2")
##'
##' ## Make a change to 'test.txt'
##' writeLines(c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'              "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
##'            con = file.path(path, "test.txt"))
##' add(repo, "test.txt")
##' commit(repo, "Second commit message branch 2")
##'
##' ## Checkout master
##' checkout(repo, "master", force = TRUE)
##'
##' ## Merge branch 1
##' merge(repo, "branch1")
##'
##' ## Merge branch 2
##' merge(repo, "branch2")
##'
##' ## Create third branch, checkout, change file and commit
##' checkout(repo, "branch3", create=TRUE)
##' writeLines(c("Lorem ipsum dolor amet sit, consectetur adipisicing elit, sed do",
##'              "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
##'            con = file.path(path, "test.txt"))
##' add(repo, "test.txt")
##' commit(repo, "Commit message branch 3")
##'
##' ## Checkout master and create a change that creates a merge conflict
##' checkout(repo, "master", force=TRUE)
##' writeLines(c("Lorem ipsum dolor sit amet, adipisicing consectetur elit, sed do",
##'              "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
##'            con = file.path(path, "test.txt"))
##' add(repo, "test.txt")
##' commit(repo, "Some commit message branch 1")
##'
##' ## Merge branch 3
##' merge(repo, "branch3")
##'
##' ## Check status; Expect to have one unstaged unmerged conflict.
##' status(repo)
##' }
setGeneric("merge",
           signature = c("object"),
           function(object, ...)
           standardGeneric("merge"))

##' @rdname merge-methods
##' @export
setMethod("merge",
          signature(object = "git_repository"),
          function(object,
                   branch,
                   commit_on_success = TRUE,
                   merger = default_signature(object))
          {
              ## Check branch argument
              if (missing(branch))
                  stop("missing 'branch' argument")
              if (any(!is.character(branch), !identical(length(branch), 1L)))
                  stop("'branch' must be a character vector of length one")

              b <- branches(object)
              b <- b[sapply(b, slot, "name") == branch][[1]]

              .Call(git2r_merge_branch,
                    b,
                    merger,
                    commit_on_success)
          }
)

##' @rdname merge-methods
##' @export
setMethod("merge",
          signature(object = "git_branch"),
          function(object,
                   commit_on_success = TRUE,
                   merger = default_signature(object@repo))
          {
              .Call(git2r_merge_branch,
                    object,
                    merger,
                    commit_on_success)
          }
)

##' Brief summary of merge result
##'
##' @aliases show,git_merge_result-methods
##' @docType methods
##' @param object The \code{\linkS4class{git_merge_result}} \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @include S4_classes.r
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user and commit a file
##' config(repo, user.name="Author", user.email="author@@example.org")
##' writeLines("First line.",
##'            file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Create and checkout a new branch. Update 'example.txt' and commit
##' checkout(repo, "new_branch", create=TRUE)
##' writeLines(c("First line.", "Second line."),
##'            file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "Second commit message")
##'
##' ## Checkout 'master' branch
##' checkout(repo, "master", force = TRUE)
##'
##' ## Display 'example.txt'
##' readLines(file.path(path, "example.txt"))
##'
##' ## Merge and display brief summary of the fast-forward merge
##' merge(repo, "new_branch")
##'
##' ## Display 'example.txt'
##' readLines(file.path(path, "example.txt"))
##' }
setMethod("show",
          signature(object = "git_merge_result"),
          function (object)
          {
              if (identical(object@up_to_date, TRUE)) {
                  cat("Already up-to-date")
              } else if (identical(object@conflicts, TRUE)) {
                  cat("Merge: Conflicts")
              } else if (identical(object@fast_forward, TRUE)) {
                  cat("Merge: Fast-forward")
              } else {
                  cat("Merge")
              }

              cat("\n")
          }
)
