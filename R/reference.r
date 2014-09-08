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

##' Get all references that can be found in a repository.
##'
##' @rdname references-methods
##' @docType methods
##' @param repo The repository \code{object}.
##' @return Character vector with references
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## List all references in repository
##' references(repo)
##' }
##'
setGeneric("references",
           signature = "repo",
           function(repo) standardGeneric("references"))

##' @rdname references-methods
##' @include S4_classes.r
##' @export
setMethod("references",
          signature(repo = "git_repository"),
          function (repo)
          {
              .Call(git2r_reference_list, repo)
          }
)

##' Brief summary of reference
##'
##' @aliases show,git_reference-methods
##' @docType methods
##' @param object The reference \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @include S4_classes.r
##' @export
setMethod("show",
          signature(object = "git_reference"),
          function (object)
          {
              if (identical(object@type, 1L)) {
                  cat(sprintf("[%s] %s\n",
                              substr(object@sha, 1 , 6),
                              object@shorthand))
              } else if (identical(object@type, 2L)) {
                  cat(sprintf("%s => %s\n",
                              object@name,
                              object@target))
              } else {
                  stop("Unexpected reference type")
              }
          }
)
