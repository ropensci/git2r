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

##' Stashes
##'
##' @rdname stashes-methods
##' @docType methods
##' @param object The repository \code{object}.
##' @return list of stashes in repository
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## List stashes in repository
##' stashes(repo)
##' }
setGeneric("stashes",
           signature = "object",
           function(object) standardGeneric("stashes"))

##' @rdname stashes-methods
##' @include repository.r
##' @export
setMethod("stashes",
          signature(object = "git_repository"),
          function (object)
          {
              .Call("stashes", object)
          }
)

##' Brief summary of a stash
##'
##' @aliases show,git_stash-methods
##' @docType methods
##' @param object The stash \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
setMethod("show",
          signature(object = "git_stash"),
          function (object)
          {
              cat(sprintf("%s\n", object@message))
          }
)

##' Summary of a stash
##'
##' @aliases summary,git_stash-methods
##' @docType methods
##' @param object The stash \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Apply summary to each stash in the repository
##' invisible(lapply(stashes(repo), summary))
##' }
setMethod("summary",
          signature(object = "git_stash"),
          function(object, ...)
          {
              cat(sprintf(paste0("message: %s\n",
                                 "stasher: %s <%s>\n",
                                 "when:    %s\n",
                                 "hex:     %s\n\n"),
                          object@message,
                          object@stasher@name,
                          object@stasher@email,
                          as(object@stasher@when, "character"),
                          object@hex))
          }
)
