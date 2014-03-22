#' @include signature.r
## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013  Stefan Widgren
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
##' @title S4 class to handle a git tag, contains \code{git_reference}.
##' @section Slots:
##' \describe{
##'   \item{sig}{
##'     An action signature
##'   }
##' }
##' @name git_tag-class
##' @aliases show,git_tag-method
##' @docType class
##' @keywords classes
##' @section Methods:
##' \describe{
##'   \item{show}{\code{signature(object = "git_tag")}}
##' }
##' @keywords methods
##' @export
setClass('git_tag',
         slots=c(sig='git_signature'),
         contains='git_reference')

##' Tags
##'
##' @name tags-methods
##' @aliases tags
##' @aliases tags-methods
##' @aliases tags,git_repository-method
##' @docType methods
##' @param object The repository \code{object}.
##' @return list of tags in repository
##' @keywords methods
##' @export
setGeneric('tags',
           signature = 'object',
           function(object) standardGeneric('tags'))

setMethod('tags',
          signature(object = 'git_repository'),
          function (object)
          {
              .Call('tags', object)
          }
)

setMethod('show',
          signature(object = 'git_tag'),
          function (object)
          {
              cat(sprintf('[%s] %s\n',
                          substr(object@hex, 1 , 6),
                          object@shorthand))
          }
)
