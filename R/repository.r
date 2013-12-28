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

##' Class \code{"repository"}
##'
##' S4 class to handle a git repository
##' @section Slots:
##' \describe{
##'   \item{repo}{
##'     External pointer to a git repository
##'   }
##' }
##' @name repository-class
##' @aliases show,repository-method
##' @docType class
##' @keywords classes
##' @section Methods:
##' \describe{
##'   \item{show}{\code{signature(object = "repository")}}
##' }
##' @keywords methods
##' @export
setClass('repository',
         slots=c(repo='externalptr'))

##' Open a repository
##'
##' @param path A path to an existing local git repository
##' @return A S4 \code{repository} object
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository('path/to/git2r')
##' }
##'
repository <- function(path) {
    ## Argument checking
    stopifnot(is.character(path),
              identical(length(path), 1L),
              nchar(path) > 0)

    path <- normalizePath(path, winslash = "/", mustWork = TRUE)
    if(!file.info(path)$isdir)
        stop('path is not a directory')

    .Call('repository', path)
}
