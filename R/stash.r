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

##' Class \code{"git_stash"}
##'
##' @title S4 class to handle a git stash
##' @section Slots:
##' \describe{
##'   \item{hex}{
##'     40 char hexadecimal string
##'   }
##'   \item{message}{
##'     Optional description of the stash
##'   }
##'   \item{stasher}{
##'     The identity of the person performing the stash
##'   }
##'   \item{repo}{
##'     The S4 class git_repository that contains the stash
##'   }
##' }
##' @name git_stash-class
##' @docType class
##' @keywords classes
##' @keywords methods
##' @include signature.r
##' @include repository.r
##' @export
setClass("git_stash",
         slots=c(hex     = "character",
                 message = "character",
                 stasher = "git_signature",
                 repo    = "git_repository"),
         validity=function(object)
         {
             errors <- validObject(object@stasher)

             if(identical(errors, TRUE))
               errors <- character()

             if(!identical(length(object@hex), 1L))
                 errors <- c(errors, "hex must have length equal to one")
             if(!identical(length(object@message), 1L))
                 errors <- c(errors, "message must have length equal to one")

             if(length(errors) == 0) TRUE else errors
         }
)
