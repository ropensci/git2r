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

##' Class \code{"git_tree"}
##'
##' @title S4 class to handle a git tree
##' @section Slots:
##' \describe{
##'   \item{hex}{
##'     40 char hexadecimal string
##'   }
##' }
##' @name git_tree-class
##' @docType class
##' @keywords classes
##' @keywords methods
##' @export
setClass('git_tree',
         slots=c(hex = 'character'),
         validity=function(object)
         {
             errors <- character(0)

             if(!identical(length(object@hex), 1L))
                 errors <- c(errors, 'hex must have length equal to one')

             if(length(errors) == 0) TRUE else errors
         }
)
