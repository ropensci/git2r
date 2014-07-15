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
##' @param repo The repository
##' @param name The remote's name
##' @param refspec The refspec to be pushed
##' @param credentials The credentials for remote repository access. Default
##' is NULL.
##' @return invisible(NULL)
##' @seealso \code{\linkS4class{cred_plaintext}},
##' \code{\linkS4class{cred_ssh_key}}
##' @keywords methods
##' @include repository.r
setGeneric("push",
           signature = "repo",
           function(repo,
                    name,
                    refspec,
                    credentials = NULL)
           standardGeneric("push"))

##' @rdname push-methods
##' @export
setMethod("push",
          signature(repo = "git_repository"),
          function (repo, name, refspec, credentials)
          {
              result <- .Call(
                  "git2r_push",
                  repo,
                  name,
                  refspec,
                  credentials,
                  "update by push",
                  default_signature(repo))

              invisible(result)
          }
)
