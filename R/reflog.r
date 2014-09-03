## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2014 The git2r contributors
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License, version 2,
## as published by the Free Software Foundation.
##
## git2r is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program; if not, write to the Free Software Foundation, Inc.,
## 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

##' List and view reflog information
##'
##' @rdname reflog-methods
##' @docType methods
##' @param repo S4 class git_repository \code{object}.
##' @param refname The name of the reference to list. 'HEAD' by
##' default.
##' @param verbose Display reflog. Default TRUE.
##' @return invisible list of S4 git_reflog_entry objects
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' reflog(repo)
##' }
##'
setGeneric("reflog",
           signature = c("repo"),
           function(repo,
                    refname = "HEAD",
                    verbose = TRUE)
           standardGeneric("reflog"))

##' @rdname reflog-methods
##' @export
setMethod("reflog",
          signature(repo = "git_repository"),
          function(repo, refname, verbose)
          {
              result <- .Call(git2r_reflog_list, repo, refname)
              if (verbose)
                  lapply(result, show)
              invisible(result)
          }
)

##' Brief summary of a reflog entry
##'
##' @aliases show,git_reflog_entry-methods
##' @docType methods
##' @param object The reflog entry \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## View repository HEAD reflog
##' reflog(repo)
##' }
##'
setMethod("show",
          signature(object = "git_reflog_entry"),
          function (object)
          {
              cat(sprintf("[%s] %s@{%i}: %s\n",
                          substring(object@hex, 1, 7),
                          object@refname,
                          object@index,
                          object@message))
          }
)
