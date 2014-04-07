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

##' Fetch
##'
##' @rdname fetch-methods
##' @docType methods
##' @param repo the repository
##' @param name the remote's name
##' @return invisible(NULL)
##' @keywords methods
##' @include repository.r
setGeneric("fetch",
           signature = "repo",
           function(repo, name) standardGeneric("fetch"))

##' @rdname fetch-methods
##' @export
setMethod("fetch",
          signature(repo = "git_repository"),
          function (repo, name)
          {
              invisible(.Call("fetch", repo, name))
          }
)
