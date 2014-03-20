#' @include time.r
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
##' @aliases show,git_signature-method
##' @docType class
##' @keywords classes
##' @keywords methods
##' @export
setClass('git_signature',
         slots=c(name='character',
                 email='character',
                 when='git_time'))

##' Brief summary of signature
##'
##' @aliases show,git_signature-methods
##' @docType methods
##' @param object The repository \code{object}
##' @return None (invisible ‘NULL’).
##' @keywords methods
##' @export
setMethod('show',
          signature(object = 'git_signature'),
          function(object)
          {
              cat(sprintf('name:  %s\nemail: %s\nwhen:  %s\n',
                          object@name,
                          object@email,
                          as(object@when, 'character')))
          }
)
