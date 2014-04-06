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

##' When
##'
##' Help method to extract the time as a character string from the S4
##' classes \code{git_commit}, \code{git_signature}, \code{git_tag}
##' and \code{git_time}.
##' @rdname when-methods
##' @docType methods
##' @param object the \code{object} to extract the time slot from.
##' @return A \code{character} vector of length one.
##' @keywords methods
##' @include commit.r
##' @include signature.r
##' @include tag.r
##' @include time.r
##' @examples \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' when(commits(repo)[[1]])
##'}
setGeneric("when",
           signature = "object",
           function(object)
           standardGeneric("when"))

##' @rdname when-methods
##' @export
setMethod("when",
          signature(object = "git_commit"),
          function (object)
          {
              as(object@author@when, "character")
          }
)

##' @rdname when-methods
##' @export
setMethod("when",
          signature(object = "git_signature"),
          function (object)
          {
              as(object@when, "character")
          }
)

##' @rdname when-methods
##' @export
setMethod("when",
          signature(object = "git_tag"),
          function (object)
          {
              as(object@tagger@when, "character")
          }
)

##' @rdname when-methods
##' @export
setMethod("when",
          signature(object = "git_time"),
          function (object)
          {
              as(object, "character")
          }
)
