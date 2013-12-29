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

##' Class \code{"reference"}
##'
##' S4 class to handle a git reference
##' @section Slots:
##' \describe{
##'   \item{name}{
##'     The full name of the reference.
##'   }
##'   \item{type}{
##'     Type of the reference, either direct (GIT_REF_OID == 1) or
##'     symbolic (GIT_REF_SYMBOLIC == 2)
##'   }
##'   \item{hex}{
##'     40 char hexadecimal string
##'   }
##'   \item{target}{
##'     :TODO:DOCUMENTATION:
##'   }
##'   \item{shorthand}{
##'     The reference's short name
##'   }
##' }
##' @name reference-class
##' @aliases show,reference-method
##' @docType class
##' @keywords classes
##' @section Methods:
##' \describe{
##'   \item{show}{\code{signature(object = "reference")}}
##' }
##' @keywords methods
##' @export
setClass('reference',
         slots=c(name='character',
                 type='integer',
                 hex='character',
                 target='character',
                 shorthand='character'),
         prototype=list(hex=NA_character_,
                        target=NA_character_))
