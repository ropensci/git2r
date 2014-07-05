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

##' Internal function to do the checkout
##'
##' @param FUN Name of git2r C function to call
##' @param object The object to checkout
##' @param force If TRUE, then make working directory match
##' target. This will throw away local changes.
##' @param ref_log_target The target in the reflog message
##' in the reflog
##' @keywords internal
do_checkout <- function(FUN, object, force, ref_log_head_spec) {
    ## Determine the one line long message to be appended to the reflog
    current <- head(object@repo)
    if(is.null(current))
        stop("Current head is NULL")
    if(is_commit(current)) {
        current <- current@hex
    } else {
        current <- current@name
    }

    msg <- sprintf("checkout: moving from %s to %s", current, ref_log_target)

    ## The identity that will used to populate the reflog
    who <- default_signature(object@repo)

    .Call(FUN, object, force, msg, who)
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
##' @include branch.r
##' @include commit.r
##' @include tag.r
##' @include tree.r
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
              ret <- do_checkout(
                  "git2r_checkout_branch",
                  object,
                  force,
                  object@name)
              invisible(ret)
          }
)

##' @rdname checkout-methods
##' @export
setMethod("checkout",
          signature(object = "git_commit"),
          function (object, force)
          {
              ret <- do_checkout(
                  "git2r_checkout_commit",
                  object,
                  force,
                  object@hex)
              invisible(ret)
          }
)

##' @rdname checkout-methods
##' @export
setMethod("checkout",
          signature(object = "git_tag"),
          function (object, force)
          {
              ret <- do_checkout(
                  "git2r_checkout_tag",
                  object,
                  force,
                  object@name)
              invisible(ret)
          }
)

##' @rdname checkout-methods
##' @export
setMethod("checkout",
          signature(object = "git_tree"),
          function (object, force)
          {
              ret <- do_checkout("git2r_checkout_tree", object, force)
              invisible(ret)
          }
)
