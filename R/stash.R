## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2019 The git2r contributors
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

stash_index <- function(object, index) {
    if (inherits(object, "git_stash")) {
        ## Determine the index of the stash in the stash list
        index <- match(object$sha, vapply(stash_list(object$repo),
                                          "[[", character(1), "sha"))
    }

    ## The stash list is zero-based
    if (abs(index - round(index)) >= .Machine$double.eps^0.5)
        stop("'index' must be an integer")
    as.integer(index) - 1L
}

stash_object <- function(object) {
    if (inherits(object, "git_stash"))
        return(object$repo)
    lookup_repository(object)
}

##' Apply stash
##'
##' Apply a single stashed state from the stash list.
##'
##' If local changes in the working directory conflict with changes in
##' the stash then an error will be raised. In this case, the index
##' will always remain unmodified and all files in the working
##' directory will remain unmodified. However, if you are restoring
##' untracked files or ignored files and there is a conflict when
##' applying the modified files, then those files will remain in the
##' working directory.
##' @param object path to a repository, or a \code{git_repository}
##'     object, or the stash \code{object} to pop. Default is a
##'     \code{path = '.'} to a reposiory.
##' @param index The index to the stash to apply. Only used when
##'     \code{object} is a path to a repository or a
##'     \code{git_repository} object. Default is \code{index = 1}.
##' @return invisible NULL
##' @export
##' @examples \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' # Configure a user
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
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
##' # Read file
##' readLines(file.path(path, "test.txt"))
##'
##' # Apply latest git_stash object in repository
##' stash_apply(stash_list(repo)[[1]])
##'
##' # Read file
##' readLines(file.path(path, "test.txt"))
##'
##' # View stashes
##' stash_list(repo)
##' }
stash_apply <- function(object = ".", index = 1) {
    .Call(git2r_stash_apply, stash_object(object), stash_index(object, index))
    invisible(NULL)
}

##' Drop stash
##'
##' @param object path to a repository, or a \code{git_repository}
##'     object, or the stash \code{object} to drop. Default is a
##'     \code{path = '.'} to a reposiory.
##' @param index The index to the stash to drop. Only used when
##'     \code{object} is a path to a repository or a
##'     \code{git_repository} object. Default is \code{index = 1}.
##' @return invisible NULL
##' @export
##' @examples \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' # Configure a user
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
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
##' stash_drop(repo, 1)
##'
##' # View stashes
##' stash_list(repo)
##' }
stash_drop <- function(object = ".", index = 1) {
    .Call(git2r_stash_drop, stash_object(object), stash_index(object, index))
    invisible(NULL)
}

##' Stash
##'
##' @template repo-param
##' @param message Optional description. Defaults to current time.
##' @param index All changes already added to the index are left
##'     intact in the working directory. Default is FALSE
##' @param untracked All untracked files are also stashed and then
##'     cleaned up from the working directory. Default is FALSE
##' @param ignored All ignored files are also stashed and then cleaned
##'     up from the working directory. Default is FALSE
##' @param stasher Signature with stasher and time of stash
##' @return invisible \code{git_stash} object if anything to stash
##'     else NULL
##' @export
##' @examples \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' # Configure a user
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
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
stash <- function(repo = ".",
                  message   = as.character(Sys.time()),
                  index     = FALSE,
                  untracked = FALSE,
                  ignored   = FALSE,
                  stasher   = NULL) {
    repo <- lookup_repository(repo)
    if (is.null(stasher))
        stasher <- default_signature(repo)
    invisible(.Call(git2r_stash_save, repo, message, index,
                    untracked, ignored, stasher))
}

##' List stashes in repository
##'
##' @template repo-param
##' @return list of stashes in repository
##' @export
##' @examples \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' # Configure a user
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
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
stash_list <- function(repo = ".") {
    .Call(git2r_stash_list, lookup_repository(repo))
}

##' Pop stash
##'
##' Apply a single stashed state from the stash list and remove it
##' from the list if successful.
##' @param object path to a repository, or a \code{git_repository}
##'     object, or the stash \code{object} to pop. Default is a
##'     \code{path = '.'} to a reposiory.
##' @param index The index to the stash to pop. Only used when
##'     \code{object} is a path to a repository or a
##'     \code{git_repository} object. Default is \code{index = 1}.
##' @return invisible NULL
##' @export
##' @examples \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' # Configure a user
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
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
##' # Read file
##' readLines(file.path(path, "test.txt"))
##'
##' # Pop latest git_stash object in repository
##' stash_pop(stash_list(repo)[[1]])
##'
##' # Read file
##' readLines(file.path(path, "test.txt"))
##'
##' # View stashes
##' stash_list(repo)
##' }
stash_pop <- function(object = ".", index = 1) {
    .Call(git2r_stash_pop, stash_object(object), stash_index(object, index))
    invisible(NULL)
}

##' @export
format.git_stash <- function(x, ...) {
    x$message
}

##' @export
print.git_stash <- function(x, ...) {
    cat(format(x, ...), "\n", sep = "")
    invisible(x)
}

##' Summary of a stash
##'
##' @param object The stash \code{object}
##' @param ... Additional arguments affecting the summary produced.
##' @return None (invisible 'NULL').
##' @export
##' @examples \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' # Configure a user
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
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
summary.git_stash <- function(object, ...) {
    cat(sprintf(paste0("message: %s\n",
                       "stasher: %s <%s>\n",
                       "when:    %s\n",
                       "sha:     %s\n\n"),
                object$summary,
                object$author$name,
                object$author$email,
                as.character(object$author$when),
                object$sha))
}
