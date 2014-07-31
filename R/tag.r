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

##' Class \code{"git_tag"}
##'
##' @title S4 class to handle a git tag
##' @section Slots:
##' \describe{
##'   \item{hex}{
##'     40 char hexadecimal string
##'   }
##'   \item{message}{
##'     The message of the tag
##'   }
##'   \item{name}{
##'     The name of the tag
##'   }
##'   \item{tagger}{
##'     The tagger (author) of the tag
##'   }
##'   \item{target}{
##'     The target of the tag
##'   }
##'   \item{repo}{
##'     The S4 class git_repository that contains the tag
##'   }
##' }
##' @name git_tag-class
##' @docType class
##' @keywords classes
##' @keywords methods
##' @include signature.r
##' @include repository.r
##' @export
setClass("git_tag",
         slots=c(hex     = "character",
                 message = "character",
                 name    = "character",
                 tagger  = "git_signature",
                 target  = "character",
                 repo    = "git_repository"),
         validity=function(object)
         {
             errors <- validObject(object@tagger)

             if (identical(errors, TRUE))
               errors <- character()

             if (!identical(length(object@hex), 1L))
                 errors <- c(errors, "hex must have length equal to one")
             if (!identical(length(object@message), 1L))
                 errors <- c(errors, "message must have length equal to one")
             if (!identical(length(object@name), 1L))
                 errors <- c(errors, "name must have length equal to one")
             if (!identical(length(object@target), 1L))
                 errors <- c(errors, "target must have length equal to one")

             if (length(errors) == 0) TRUE else errors
         }
)

##' Tag
##'
##' @rdname tag-methods
##' @docType methods
##' @param object The repository \code{object}.
##' @param name Name for the tag.
##' @param message The tag message.
##' @param tagger The tagger (author) of the tag
##' @return invisible(\code{git_tag}) object
##' @keywords methods
setGeneric("tag",
           signature = "object",
           function(object,
                    name,
                    message,
                    tagger = default_signature(object))
           standardGeneric("tag"))

##' @rdname tag-methods
##' @export
setMethod("tag",
          signature(object = "git_repository"),
          function (object,
                    name,
                    message,
                    tagger)
          {
              ## Argument checking
              stopifnot(is.character(name),
                        identical(length(name), 1L),
                        nchar(name[1]) > 0,
                        is.character(message),
                        identical(length(message), 1L),
                        nchar(message[1]) > 0,
                        is(tagger, "git_signature"))

              invisible(.Call("git2r_tag_create", object, name, message, tagger))
          }
)

##' Tags
##'
##' @rdname tags-methods
##' @docType methods
##' @param repo The repository
##' @return list of tags in repository
##' @keywords methods
setGeneric("tags",
           signature = "repo",
           function(repo)
           standardGeneric("tags"))

##' @rdname tags-methods
##' @export
setMethod("tags",
          signature(repo = "git_repository"),
          function (repo)
          {
              .Call("git2r_tag_list", repo)
          }
)

##' Brief summary of a tag
##'
##' @aliases show,git_tag-methods
##' @docType methods
##' @param object The tag \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
setMethod("show",
          signature(object = "git_tag"),
          function (object)
          {
              cat(sprintf("[%s] %s\n",
                          substr(object@target, 1 , 6),
                          object@name))
          }
)

##' Summary of a tag
##'
##' @aliases summary,git_tag-methods
##' @docType methods
##' @param object The tag \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Apply summary to each tag in the repository
##' invisible(lapply(tags(repo), summary))
##' }
setMethod("summary",
          signature(object = "git_tag"),
          function(object, ...)
          {
              cat(sprintf(paste0("name:    %s\n",
                                 "target:  %s\n",
                                 "tagger:  %s <%s>\n",
                                 "when:    %s\n",
                                 "message: %s\n"),
                          object@name,
                          object@target,
                          object@tagger@name,
                          object@tagger@email,
                          as(object@tagger@when, "character"),
                          object@message))
          }
)
