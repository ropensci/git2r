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

##' Checkout
##'
##' Update files in the and working tree to match the content of the
##' tree pointed at by the treeish.
##' @rdname checkout-methods
##' @docType methods
##' @param repo The repository.
##' @param treeish a commit, tag or tree which content will be used to
##' update the working directory or the default to use 'HEAD'.
##' @return invisible NULL
##' @keywords methods
##' @include repository.r
##' @include commit.r
##' @include tag.r
setGeneric('checkout',
           signature = 'repo',
           function(repo,
                    treeish = 'HEAD')
           standardGeneric('checkout')
)

##' @rdname checkout-methods
##' @export
setMethod('checkout',
          signature(repo = 'git_repository'),
          function (repo, treeish)
          {
              if(isS4(treeish)) {
                  if(!any(is(treeish, 'git_commit'),
                          is(treeish, 'git_tag'),
                          is(treeish, 'git_tree'))) {
                      stop('treeish must be a commit, tag, tree or HEAD')
                  }
              } else if(!identical(treeish, 'HEAD')) {
                  stop('treeish must be a commit, tag, tree or HEAD')
              }

              invisible(.Call('checkout', repo, treeish))
          }
)
