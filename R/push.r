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

##' Push
##'
##' @rdname push-methods
##' @docType methods
##' @param object S4 class \code{git_repository} or \code{git_branch}.
##' @param ... Additional arguments affecting the push.
##' @param credentials The credentials for remote repository
##' access. Default is NULL.
##' @return invisible(NULL)
##' @seealso \code{\linkS4class{cred_plaintext}},
##' \code{\linkS4class{cred_ssh_key}}
##' @keywords methods
##' @include S4-classes.r
##' @examples \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Make some changes and commit...
##'
##' ## push changes to remote
##' push(head(repo))
##' }
setGeneric("push",
           signature = "object",
           function(object, ...)
           standardGeneric("push"))

##' @rdname push-methods
##' @export
setMethod("push",
          signature(object = "git_branch"),
          function (object,
                    credentials = NULL)
          {
              upstream <- branch_get_upstream(object)
              if (is.null(upstream)) {
                  stop(paste0("The branch '", object@name, "' that you are ",
                              "trying to push does not track an upstream branch."))
              }

              src <- .Call(git2r_branch_canonical_name, object)
              dst <- .Call(git2r_branch_upstream_canonical_name, object)

              push(object      = object@repo,
                   name        = branch_remote_name(upstream),
                   refspec     = paste0(src, ":", dst),
                   credentials = credentials)
          }
)

##' @rdname push-methods
##' @param name The remote's name. Default is NULL.
##' @param refspec The refspec to be pushed. Default is NULL.
##' @export
setMethod("push",
          signature(object = "git_repository"),
          function (object,
                    name        = NULL,
                    refspec     = NULL,
                    credentials = NULL)
          {
              if (all(is.null(name), is.null(refspec))) {
                  stop("Push when both name and refspec equals NULL isn't implemented. Sorry")

                  ##
                  ## :TODO:FIXME: Read remote's name and refspec from
                  ## config
                  ##

              } else if (any(is.null(name), is.null(refspec))) {
                  stop("Both 'name' and 'refspec' must be 'character' or 'NULL'")
              }

              result <- .Call(
                  git2r_push,
                  object,
                  name,
                  refspec,
                  credentials,
                  "update by push",
                  default_signature(object))

              invisible(result)
          }
)
