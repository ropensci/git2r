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

##' Class \code{"git_blob"}
##'
##' @title  S4 class to handle a git blob
##' @section Slots:
##' \describe{
##'   \item{hex}{
##'     40 char hexadecimal string
##'   }
##'   \item{path}{
##'     Path to the git repository that contains the blob
##'   }
##' }
##' @rdname git_blob-class
##' @docType class
##' @keywords classes
##' @keywords methods
##' @export
setClass("git_blob",
         slots=c(hex  = "character",
                 path = "character"),
         validity=function(object) {
             errors <- character()

             if (length(errors) == 0) TRUE else errors
         }
)

##' Is blob binary
##'
##' @rdname is.binary-methods
##' @docType methods
##' @param object The blob \code{object}.
##' @return TRUE if binary data, FALSE if not.
##' @keywords methods
setGeneric("is.binary",
           signature = "object",
           function(object)
           standardGeneric("is.binary"))

##' @rdname is.binary-methods
##' @export
setMethod("is.binary",
          signature(object = "git_blob"),
          function (object)
          {
              .Call("is_binary", object)
          }
)
