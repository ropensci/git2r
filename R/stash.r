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

##' Class \code{"git_stash"}
##'
##' @title S4 class to handle a git stash
##' @name git_stash-class
##' @docType class
##' @keywords classes
##' @keywords methods
##' @include commit.r
##' @export
setClass("git_stash", contains = "git_commit")

##' Stash
##'
##' @rdname stash-methods
##' @docType methods
##' @param object The repository \code{object}.
##' @param message Optional description. Defaults to current time.
##' @param index All changes already added to the index are left
##' intact in the working directory. Default is FALSE
##' @param untracked All untracked files are also stashed and then
##' cleaned up from the working directory. Default is FALSE
##' @param ignored All ignored files are also stashed and then cleaned
##' up from the working directory. Default is FALSE
##' @param stasher Signature with stasher and time of stash
##' @return invisible S4 class git_stash if anything to stash else NULL
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Create stash in repository
##' stash(repo, "Stash message")
##' }
setGeneric("stash",
           signature = "object",
           function(object,
                    message   = as.character(Sys.time()),
                    index     = FALSE,
                    untracked = FALSE,
                    ignored   = FALSE,
                    stasher   = default_signature(object))
           standardGeneric("stash"))

##' @rdname stash-methods
##' @include repository.r
##' @export
setMethod("stash",
          signature(object = "git_repository"),
          function (object,
                    message,
                    index,
                    untracked,
                    ignored,
                    stasher)
          {
              invisible(.Call("stash",
                              object,
                              message,
                              index,
                              untracked,
                              ignored,
                              stasher))
          }
)

##' Stashes
##'
##' @rdname stashes-methods
##' @docType methods
##' @param object The repository \code{object}.
##' @return list of stashes in repository
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## List stashes in repository
##' stashes(repo)
##' }
setGeneric("stashes",
           signature = "object",
           function(object) standardGeneric("stashes"))

##' @rdname stashes-methods
##' @include repository.r
##' @export
setMethod("stashes",
          signature(object = "git_repository"),
          function (object)
          {
              .Call("stashes", object)
          }
)

##' Brief summary of a stash
##'
##' @aliases show,git_stash-methods
##' @docType methods
##' @param object The stash \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
setMethod("show",
          signature(object = "git_stash"),
          function (object)
          {
              cat(sprintf("%s\n", object@message))
          }
)

##' Summary of a stash
##'
##' @aliases summary,git_stash-methods
##' @docType methods
##' @param object The stash \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Apply summary to each stash in the repository
##' invisible(lapply(stashes(repo), summary))
##' }
setMethod("summary",
          signature(object = "git_stash"),
          function(object, ...)
          {
              cat(sprintf(paste0("message: %s\n",
                                 "stasher: %s <%s>\n",
                                 "when:    %s\n",
                                 "hex:     %s\n\n"),
                          object@summary,
                          object@author@name,
                          object@author@email,
                          as(object@author@when, "character"),
                          object@hex))
          }
)
