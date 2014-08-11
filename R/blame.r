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

##' Get blame for file
##'
##' @rdname blame-methods
##' @docType methods
##' @param repo The repository
##' @param path Path to the file to consider
##' @return S4 class git_blame object
##' @keywords methods
setGeneric("blame",
           signature = c("repo", "path"),
           function(repo,
                    path)
           standardGeneric("blame"))

##' @rdname blame-methods
##' @export
setMethod("blame",
          signature(repo = "git_repository",
                    path = "character"),
          function(repo,
                   path)
          {
              .Call(git2r_blame_file, repo, path)
          }
)
