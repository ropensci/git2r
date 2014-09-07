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

##' Drop stash
##'
##' @rdname stash_drop-methods
##' @docType methods
##' @param object The stash \code{object} to drop or a zero-based
##' integer to the stash to drop. The last stash has index 0.
##' @param index Zero based index to the stash to drop.
##' @return invisible NULL
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Assuming there are stashes in the repository.
##' ## Drop a stash in repository.
##' stash_drop(stashes(repo)[[1]])
##'
##' ## Assuming there are stashes in the repository.
##' ## Drop last stash in repository.
##' stash_drop(repo, 0)
##' }
setGeneric("stash_drop",
           signature = c("object", "index"),
           function(object, index)
           standardGeneric("stash_drop"))

##' @rdname stash_drop-methods
##' @include S4_classes.r
##' @export
setMethod("stash_drop",
          signature(object = "git_repository",
                    index  = "integer"),
          function (object, index)
          {
              invisible(.Call(git2r_stash_drop, object, index))
          }
)

##' @rdname stash_drop-methods
##' @include S4_classes.r
##' @export
setMethod("stash_drop",
          signature(object = "git_repository",
                    index  = "numeric"),
          function (object, index)
          {
              if (abs(index - round(index)) >= .Machine$double.eps^0.5)
                  stop("'index' must be an integer")
              stash_drop(object, as.integer(index));
          }
)

##' @rdname stash_drop-methods
##' @export
setMethod("stash_drop",
          signature(object = "git_stash",
                    index  = "missing"),
          function (object)
          {
              ## Determine the index of the stash in the stash list
              i <- match(object@sha, sapply(stashes(object@repo), slot, "sha"))

              ## The stash list is zero-based
              stash_drop(object@repo, i-1L);
          }
)

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
##' @include S4_classes.r
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
              invisible(.Call(git2r_stash_save,
                              object,
                              message,
                              index,
                              untracked,
                              ignored,
                              stasher))
          }
)

##' List stashes in repository
##'
##' @rdname stash_list-methods
##' @docType methods
##' @param repo The repository.
##' @return list of stashes in repository
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## List stashes in repository
##' stash_list(repo)
##' }
setGeneric("stash_list",
           signature = "repo",
           function(repo)
           standardGeneric("stash_list"))

##' @rdname stash_list-methods
##' @include S4_classes.r
##' @export
setMethod("stash_list",
          signature(repo = "git_repository"),
          function(repo)
          {
              .Call(git2r_stash_list, repo)
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
                                 "sha:     %s\n\n"),
                          object@summary,
                          object@author@name,
                          object@author@email,
                          as(object@author@when, "character"),
                          object@sha))
          }
)
