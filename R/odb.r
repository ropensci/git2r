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

##' List all objects available in the database
##'
##' @rdname odb_list-methods
##' @docType methods
##' @param repo The repository
##' @return list of S4 class git_object \code{objects}
##' @keywords methods
setGeneric("odb_list",
           signature = "repo",
           function(repo)
           standardGeneric("odb_list"))

##' @rdname odb_list-methods
##' @export
setMethod("odb_list",
          signature(repo = "git_repository"),
          function(repo)
          {
              .Call(git2r_odb_list, repo)
          }
)
