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

##' Class \code{"git_signature"}
##'
##' @title S4 class to handle a git signature
##' @section Slots:
##' \describe{
##'   \item{name}{
##'     The full name of the author.
##'   }
##'   \item{email}{
##'     Email of the author.
##'   }
##'   \item{when}{
##'     Time when the action happened.
##'   }
##' }
##' @name git_signature-class
##' @docType class
##' @keywords classes
##' @keywords methods
##' @include time.r
##' @export
setClass("git_signature",
         slots=c(name="character",
                 email="character",
                 when="git_time"),
         validity=function(object)
         {
             errors <- validObject(object@when)

             if (identical(errors, TRUE))
               errors <- character()

             if (!identical(length(object@name), 1L))
                 errors <- c(errors, "name must have length equal to one")
             if (!identical(length(object@email), 1L))
                 errors <- c(errors, "email must have length equal to one")

             if (length(errors) == 0) TRUE else errors
         }
)

##' Brief summary of signature
##'
##' @aliases show,git_signature-methods
##' @docType methods
##' @param object The repository \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
setMethod("show",
          signature(object = "git_signature"),
          function(object)
          {
              cat(sprintf(paste0("name:  %s\n",
                                 "email: %s\n",
                                 "when:  %s\n"),
                          object@name,
                          object@email,
                          as(object@when, "character")))
          }
)
