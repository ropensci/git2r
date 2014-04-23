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

##' Class \code{"git_tree"}
##'
##' @title S4 class to handle a git tree
##' @section Slots:
##' \describe{
##'   \item{hex}{
##'     40 char hexadecimal string
##'   }
##'   \item{filemode}{
##'     :TODO:DOCUMENTATION:
##'   }
##'   \item{type}{
##'     :TODO:DOCUMENTATION:
##'   }
##'   \item{id}{
##'     :TODO:DOCUMENTATION:
##'   }
##'   \item{name}{
##'     :TODO:DOCUMENTATION:
##'   }
##'   \item{repo}{
##'     The S4 class git_repository that contains the commit
##'   }
##' }
##' @name git_tree-class
##' @docType class
##' @keywords classes
##' @keywords methods
##' @include repository.r
##' @export
setClass("git_tree",
         slots=c(hex      = "character",
                 filemode = "integer",
                 type     = "character",
                 id       = "character",
                 name     = "character",
                 repo     = "git_repository"),
         validity=function(object)
         {
             errors <- character(0)

             if(!identical(length(object@hex), 1L))
                 errors <- c(errors, "hex must have length equal to one")

             if(length(errors) == 0) TRUE else errors
         }
)

##' Tree
##'
##' Get the tree pointed to by a commit.
##' @rdname tree-methods
##' @docType methods
##' @param object the \code{commit} object
##' @return A S4 class git_tree object
##' @keywords methods
##' @include commit.r
setGeneric("tree",
           signature = "object",
           function(object) standardGeneric("tree"))

##' @rdname tree-methods
##' @export
setMethod("tree",
          signature(object = "git_commit"),
          function (object)
          {
              .Call("commit_tree", object)
          }
)
