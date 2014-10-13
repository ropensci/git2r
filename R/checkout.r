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
##' @param object a commit, tag or tree which content will be used to
##' update the working directory.
##' @param force If TRUE, then make working directory match
##' target. This will throw away local changes. Default is FALSE.
##' @return invisible NULL
##' @keywords methods
##' @include S4_classes.r
setGeneric("checkout",
           signature = "object",
           function (object, force = FALSE)
           standardGeneric("checkout")
)

##' @rdname checkout-methods
##' @export
setMethod("checkout",
          signature(object = "git_branch"),
          function (object, force)
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
          function (object, force)
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
          function (object, force)
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
