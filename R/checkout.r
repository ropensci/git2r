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

##' Internal function to generate checkout reflog message
##'
##' @param object The object to checkout
##' @param ref_log_target The target in the reflog message
##' in the reflog
##' @keywords internal
checkout_reflog_msg <- function(object, ref_log_target) {
    ## Determine the one line long message to be appended to the reflog
    current <- head(object@repo)
    if (is.null(current))
        stop("Current head is NULL")
    if (is_commit(current)) {
        current <- current@sha
    } else {
        current <- current@name
    }

    sprintf("checkout: moving from %s to %s", current, ref_log_target)
}

##' Checkout
##'
##' Update files in the index and working tree to match the content of
##' the tree pointed at by the treeish object (commit, tag or tree).
##' Checkout using the default GIT_CHECKOUT_SAFE_CREATE strategy
##' (force = FALSE) or GIT_CHECKOUT_FORCE (force = TRUE).
##' @rdname checkout-methods
##' @docType methods
##' @param object a repository, commit, tag or tree which content will
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
                      checkout(branch, force = TRUE)
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
              ret <- .Call(
                  git2r_checkout_branch,
                  object,
                  force,
                  checkout_reflog_msg(object, object@name),
                  default_signature(object@repo))
              invisible(ret)
          }
)

##' @rdname checkout-methods
##' @export
setMethod("checkout",
          signature(object = "git_commit"),
          function (object, force = FALSE)
          {
              ret <- .Call(
                  git2r_checkout_commit,
                  object,
                  force,
                  checkout_reflog_msg(object, object@sha),
                  default_signature(object@repo))
              invisible(ret)
          }
)

##' @rdname checkout-methods
##' @export
setMethod("checkout",
          signature(object = "git_tag"),
          function (object, force = FALSE)
          {
              ret <- .Call(
                  git2r_checkout_tag,
                  object,
                  force,
                  checkout_reflog_msg(object, object@name),
                  default_signature(object@repo))
              invisible(ret)
          }
)
