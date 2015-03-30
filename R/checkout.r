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

##' Checkout
##'
##' Update files in the index and working tree to match the content of
##' the tree pointed at by the treeish object (commit, tag or tree).
##' Checkout using the default GIT_CHECKOUT_SAFE_CREATE strategy
##' (force = FALSE) or GIT_CHECKOUT_FORCE (force = TRUE).
##' @rdname checkout-methods
##' @docType methods
##' @param object A repository, commit, tag or tree which content will
##' be used to update the working directory.
##' @param ... Additional arguments affecting the checkout
##' @param branch If object is a repository, the name of the branch to
##' check out.
##' @param create If object is a repository, then branch is created if
##' doesn't exist.
##' @param force If TRUE, then make working directory match
##' target. This will throw away local changes. Default is FALSE.
##' @return invisible NULL
##' @keywords methods
##' @include S4_classes.r
##' @examples
##' \dontrun{
##' ## Create directories and initialize repositories
##' path_bare <- tempfile(pattern="git2r-")
##' path_repo_1 <- tempfile(pattern="git2r-")
##' path_repo_2 <- tempfile(pattern="git2r-")
##' dir.create(path_bare)
##' dir.create(path_repo_1)
##' dir.create(path_repo_2)
##' repo_bare <- init(path_bare, bare = TRUE)
##'
##' ## Clone to repo 1 and config user
##' repo_1 <- clone(path_bare, path_repo_1)
##' config(repo_1, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Add changes to repo 1 and push to bare
##' writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'            con = file.path(path_repo_1, "test.txt"))
##' add(repo_1, "test.txt")
##' commit(repo_1, "First commit message")
##' push(repo_1, "origin", "refs/heads/master")
##'
##' ## Create and checkout 'dev' branch in repo 1
##' checkout(repo_1, "dev", create = TRUE)
##'
##' ## Add changes to 'dev' branch in repo 1 and push to bare
##' writeLines(c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'              "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
##'            con = file.path(path_repo_1, "test.txt"))
##' add(repo_1, "test.txt")
##' commit(repo_1, "Second commit message")
##' push(repo_1, "origin", "refs/heads/dev")
##'
##' ## Clone to repo 2
##' repo_2 <- clone(path_bare, path_repo_2)
##' config(repo_2, user.name="Bob", user.email="bob@@example.org")
##'
##' ## Read content of 'test.txt'
##' readLines(file.path(path_repo_2, "test.txt"))
##'
##' ## Checkout dev branch
##' checkout(repo_2, "dev")
##'
##' ## Read content of 'test.txt'
##' readLines(file.path(path_repo_2, "test.txt"))
##' }
setGeneric("checkout",
           signature = "object",
           function (object, ...)
           standardGeneric("checkout")
)

##' @rdname checkout-methods
##' @export
setMethod("checkout",
          signature(object = "git_repository"),
          function (object, branch, create = FALSE, force = FALSE)
          {
              ## Check branch argument
              if (missing(branch))
                  stop("missing 'branch' argument")
              if (any(!is.character(branch), !identical(length(branch), 1L)))
                  stop("'branch' must be a character vector of length one")

              if (identical(branch, "-")) {
                  ## Determine previous branch name
                  branch <- revparse_single(object, "@{-1}")@sha
                  branch <- sapply(references(object), function(x) {
                      ifelse(x@sha == branch, x@shorthand, NA_character_)
                  })
                  branch <- branch[!sapply(branch, is.na)]
                  branch <- sapply(branches(object, "local"), function(x) {
                      ifelse(x@name %in% branch, x@name, NA_character_)
                  })
                  branch <- branch[!sapply(branch, is.na)]
                  if (any(!is.character(branch), !identical(length(branch), 1L)))
                      stop("'branch' must be a character vector of length one")
              }

              ## Check if branch exists in a local branch
              lb <- branches(object, "local")
              lb <- lb[sapply(lb, slot, "name") == branch]
              if (length(lb)) {
                  checkout(lb[[1]], force = force)
              } else {
                  ## Check if there exists exactly one remote branch
                  ## with a matching name.
                  rb <- branches(object, "remote")

                  ## Split remote/name to check for a unique name
                  name <- sapply(rb, function(x) {
                      remote <- strsplit(x@name, "/")[[1]][1]
                      sub(paste0("^", remote, "/"), "", x@name)
                  })
                  i <- which(name == branch)
                  if (identical(length(i), 1L)) {
                      ## Create branch and track remote
                      commit <- lookup(object, branch_target(rb[[i]]))
                      branch <- branch_create(commit, branch)
                      branch_set_upstream(branch, rb[[i]]@name)
                      checkout(branch, force = force)
                  } else {
                      if (!identical(create, TRUE))
                          stop(sprintf("'%s' did not match any branch", branch))

                      ## Create branch
                      commit <- lookup(object, branch_target(head(object)))
                      checkout(branch_create(commit, branch), force = force)
                  }
              }

              invisible(NULL)
          }
)

##' @rdname checkout-methods
##' @export
setMethod("checkout",
          signature(object = "git_branch"),
          function (object, force = FALSE)
          {
              ref_name <- paste0("refs/heads/", object@name)
              .Call(git2r_checkout_tree, object@repo, ref_name, force)
              .Call(git2r_repository_set_head, object@repo, ref_name)
              invisible(NULL)
          }
)

##' @rdname checkout-methods
##' @export
setMethod("checkout",
          signature(object = "git_commit"),
          function (object, force = FALSE)
          {
              .Call(git2r_checkout_tree, object@repo, object@sha, force)
              .Call(git2r_repository_set_head_detached, object)
              invisible(NULL)
          }
)

##' @rdname checkout-methods
##' @export
setMethod("checkout",
          signature(object = "git_tag"),
          function (object, force = FALSE)
          {
              .Call(git2r_checkout_tree, object@repo, object@target, force)
              .Call(git2r_repository_set_head_detached,
                    lookup(repo, object@target))
              invisible(NULL)
          }
)
