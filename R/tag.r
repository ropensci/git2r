## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2014  Stefan Widgren
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, version 2 of the License.
##
## git2r is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.

##' Class \code{"git_tag"}
##'
##' @title S4 class to handle a git tag
##' @section Slots:
##' \describe{
##'   \item{name}{
##'     The name of the tag
##'   }
##'   \item{message}{
##'     The message of the tag
##'   }
##'   \item{tagger}{
##'     The tagger (author) of the tag
##'   }
##'   \item{target}{
##'     The target of the tag
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
         slots=c(message = "character",
                 name    = "character",
                 tagger  = "git_signature",
                 target  = "character"),
         validity=function(object)
         {
             errors <- validObject(object@tagger)

             if(identical(errors, TRUE))
               errors <- character()

             if(!identical(length(object@name), 1L))
                 errors <- c(errors, "name must have length equal to one")
             if(!identical(length(object@message), 1L))
                 errors <- c(errors, "message must have length equal to one")
             if(!identical(length(object@target), 1L))
                 errors <- c(errors, "target must have length equal to one")

             if(length(errors) == 0) TRUE else errors
         }
)

##' Tags
##'
##' @rdname tags-methods
##' @docType methods
##' @param object The repository \code{object}.
##' @return list of tags in repository
##' @keywords methods
##' @export
setGeneric("tags",
           signature = "object",
           function(object) standardGeneric("tags"))

##' @rdname tags-methods
##' @export
setMethod("tags",
          signature(object = "git_repository"),
          function (object)
          {
              .Call("tags", object)
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
