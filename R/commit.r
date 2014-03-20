#' @include tag.r
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

##' Class \code{"git_commit"}
##'
##' @title S4 class to handle a git commit.
##' @section Slots:
##' \describe{
##'   \item{hex}{
##'     40 char hexadecimal string
##'   }
##'   \item{author}{
##'     An author signature
##'   }
##'   \item{committer}{
##'     The committer signature
##'   }
##'   \item{summary}{
##'     The short "summary" of a git commit message, comprising the first
##'     paragraph of the message with whitespace trimmed and squashed.
##'   }
##'   \item{message}{
##'     The message of a commit
##'   }
##' }
##' @name git_commit-class
##' @aliases show,git_commit-method
##' @docType class
##' @keywords classes
##' @section Methods:
##' \describe{
##'   \item{show}{\code{signature(object = "git_commit")}}
##' }
##' @keywords methods
##' @export
setClass('git_commit',
         slots=c(hex='character',
                 author='git_signature',
                 committer='git_signature',
                 summary='character',
                 message='character'),
         prototype=list(summary=NA_character_,
                        message=NA_character_))

##' Commits
##'
##' @name commits-methods
##' @aliases commits
##' @aliases commits-methods
##' @aliases commits,git_repository-method
##' @docType methods
##' @param object :TODO:DOCUMENTATION:
##' @return list of commits in repository
##' @keywords methods
##' @export
setGeneric('commits',
           signature = 'object',
           function(object) standardGeneric('commits'))

setMethod('commits',
          signature(object = 'git_repository'),
          function (object)
          {
              .Call('revisions', object)
          }
)

setMethod('show',
          signature(object = 'git_commit'),
          function (object)
          {
              cat(sprintf('Commit:  %s\nAuthor:  %s <%s>\nWhen:    %s\nSummary: %s\n',
                          object@hex,
                          object@author@name,
                          object@author@email,
                          as(object@author@when, 'character'),
                          object@summary))
          }
)
