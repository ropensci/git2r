## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2015 The git2r contributors
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
##' @param ... Additional arguments affecting the stash_drop
##' @param index Zero based index to the stash to drop. Only used when
##' \code{object} is a \code{git_repository}.
##' @return invisible NULL
##' @keywords methods
##' @examples \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' # Configure a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' # Create a file, add and commit
##' writeLines("Hello world!", file.path(path, "test.txt"))
##' add(repo, 'test.txt')
##' commit(repo, "Commit message")
##'
##' # Change file
##' writeLines(c("Hello world!", "HELLO WORLD!"), file.path(path, "test.txt"))
##'
##' # Create stash in repository
##' stash(repo)
##'
##' # Change file
##' writeLines(c("Hello world!", "HeLlO wOrLd!"), file.path(path, "test.txt"))
##'
##' # Create stash in repository
##' stash(repo)
##'
##' # View stashes
##' stash_list(repo)
##'
##' # Drop git_stash object in repository
##' stash_drop(stash_list(repo)[[1]])
##'
##' ## Drop stash using an index to stash
##' stash_drop(repo, 0)
##'
##' # View stashes
##' stash_list(repo)
##' }
setGeneric("stash_drop",
           signature = "object",
           function(object, ...)
           standardGeneric("stash_drop"))

##' @rdname stash_drop-methods
##' @include S4_classes.r
##' @export
setMethod("stash_drop",
          signature(object = "git_repository"),
          function(object, index)
          {
              if (missing(index))
                  stop("missing argument 'index'")
              if (abs(index - round(index)) >= .Machine$double.eps^0.5)
                  stop("'index' must be an integer")
              index <- as.integer(index)
              .Call(git2r_stash_drop, object, index)
              invisible(NULL)
          }
)

##' @rdname stash_drop-methods
##' @export
setMethod("stash_drop",
          signature(object = "git_stash"),
          function(object)
          {
              ## Determine the index of the stash in the stash list
              i <- match(object@sha, sapply(stash_list(object@repo), slot, "sha"))

              ## The stash list is zero-based
              .Call(git2r_stash_drop, object@repo, i - 1L)
              invisible(NULL)
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
##' @examples \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' # Configure a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' # Create a file, add and commit
##' writeLines("Hello world!", file.path(path, "test.txt"))
##' add(repo, 'test.txt')
##' commit(repo, "Commit message")
##'
##' # Change file
##' writeLines(c("Hello world!", "HELLO WORLD!"), file.path(path, "test.txt"))
##'
##' # Check status of repository
##' status(repo)
##'
##' # Create stash in repository
##' stash(repo)
##'
##' # Check status of repository
##' status(repo)
##'
##' # View stash
##' stash_list(repo)
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
          function(object,
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
##' @param repo The repository \code{object}
##' \code{\linkS4class{git_repository}}. If the \code{repo} argument
##' is missing, the repository is searched for with
##' \code{\link{discover_repository}} in the current working
##' directory.
##' @return list of stashes in repository
##' @keywords methods
##' @include S4_classes.r
##' @examples \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' # Configure a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' # Create a file, add and commit
##' writeLines("Hello world!", file.path(path, "test-1.txt"))
##' add(repo, 'test-1.txt')
##' commit(repo, "Commit message")
##'
##' # Make one more commit
##' writeLines(c("Hello world!", "HELLO WORLD!"), file.path(path, "test-1.txt"))
##' add(repo, 'test-1.txt')
##' commit(repo, "Next commit message")
##'
##' # Create one more file
##' writeLines("Hello world!", file.path(path, "test-2.txt"))
##'
##' # Check that there are no stashes
##' stash_list(repo)
##'
##' # Stash
##' stash(repo)
##'
##' # Only untracked changes, therefore no stashes
##' stash_list(repo)
##'
##' # Stash and include untracked changes
##' stash(repo, "Stash message", untracked=TRUE)
##'
##' # View stash
##' stash_list(repo)
##' }
setGeneric("stash_list",
           signature = "repo",
           function(repo)
           standardGeneric("stash_list"))

##' @rdname stash_list-methods
##' @export
setMethod("stash_list",
          signature(repo = "missing"),
          function()
          {
              callGeneric(repo = lookup_repository())
          }
)

##' @rdname stash_list-methods
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
##' @examples \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' # Configure a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' # Create a file, add and commit
##' writeLines("Hello world!", file.path(path, "test.txt"))
##' add(repo, 'test.txt')
##' commit(repo, "Commit message")
##'
##' # Change file
##' writeLines(c("Hello world!", "HELLO WORLD!"), file.path(path, "test.txt"))
##'
##' # Create stash in repository
##' stash(repo, "Stash message")
##'
##' # View brief summary of stash
##' stash_list(repo)[[1]]
##' }
setMethod("show",
          signature(object = "git_stash"),
          function(object)
          {
              cat(sprintf("%s\n", object@message))
          }
)

##' Summary of a stash
##'
##' @aliases summary,git_stash-methods
##' @docType methods
##' @param object The stash \code{object}
##' @param ... Additional arguments affecting the summary produced.
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
##' @examples \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' # Configure a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' # Create a file, add and commit
##' writeLines("Hello world!", file.path(path, "test.txt"))
##' add(repo, 'test.txt')
##' commit(repo, "Commit message")
##'
##' # Change file
##' writeLines(c("Hello world!", "HELLO WORLD!"), file.path(path, "test.txt"))
##'
##' # Create stash in repository
##' stash(repo, "Stash message")
##'
##' # View summary of stash
##' summary(stash_list(repo)[[1]])
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
