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

##' Pull
##'
##' @rdname pull-methods
##' @docType methods
##' @param repo the repository
##' @return invisible(NULL)
##' @keywords methods
##' @include repository.r
setGeneric('pull',
           signature = 'repo',
           function(repo) standardGeneric('pull'))

##' @rdname pull-methods
##' @export
setMethod('pull',
          signature(repo = 'git_repository'),
          function (repo)
          {
              invisible(NULL)
          }
)
