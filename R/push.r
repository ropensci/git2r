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
##' @param repo the repository
##' @param name the remote's name
##' @param refspec the refspec to be pushed
##' @return invisible(NULL)
##' @keywords methods
##' @include repository.r
setGeneric("push",
           signature = "repo",
           function(repo, name, refspec)
           standardGeneric("push"))

##' @rdname push-methods
##' @export
setMethod("push",
          signature(repo = "git_repository"),
          function (repo, name, refspec)
          {
              invisible(.Call("git2r_push", repo, name, refspec))
          }
)
