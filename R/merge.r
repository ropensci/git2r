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
##'
##' ## Cleanup
##' unlink(path, recursive=TRUE)
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
##' @param object A \code{\linkS4class{git_branch}} object to merge
##' into HEAD.
##' @param ... Additional arguments affecting the merge
##' @return A \code{\linkS4class{git_merge_result}} object.
##' @keywords methods
##' @include S4_classes.r
setGeneric("merge",
           signature = c("object"),
           function(object, ...)
           standardGeneric("merge"))

##' @rdname merge-methods
##' @param commit_on_success If there are no conflicts written to the
##' index, the merge commit will be committed. Default is TRUE.
##' @param merger Who made the merge.
##' @export
setMethod("merge",
          signature(object = "git_branch"),
          function(object,
                   commit_on_success = TRUE,
                   merger = default_signature(repo))
          {
              .Call(git2r_merge_branch,
                    object,
                    merger,
                    commit_on_success)
          }
)
