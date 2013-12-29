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

##' Class \code{"tag"}
##'
##' S4 class to handle a git tag, contains \code{\link{reference}}.
##' @name tag-class
##' @aliases show,tag-method
##' @docType class
##' @keywords classes
##' @section Methods:
##' \describe{
##'   \item{show}{\code{signature(object = "tag")}}
##' }
##' @keywords methods
##' @export
setClass('tag',
         contains='reference')

##' Tags
##'
##' @name tags-methods
##' @aliases tags
##' @aliases tags-methods
##' @aliases tags,repository-method
##' @docType methods
##' @param object :TODO:DOCUMENTATION:
##' @return list of tags in repository
##' @keywords methods
##' @export
setGeneric('tags',
           signature = 'object',
           function(object) standardGeneric('tags'))

setMethod('tags',
          signature(object = 'repository'),
          function (object)
          {
              .Call('tags', object)
          }
)

setMethod('show',
          signature(object = 'tag'),
          function (object)
          {
              cat(sprintf('[%s] %s\n',
                          substr(object@hex, 1 , 6),
                          object@shorthand))
          }
)
