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

##' Class \code{"git_repository"}
##'
##' @title  S4 class to handle a git repository
##' @section Slots:
##' \describe{
##'   \item{path}{
##'     Path to a git repository
##'   }
##' }
##' @rdname git_repository-class
##' @docType class
##' @keywords classes
##' @section Methods:
##' \describe{
##'   \item{is.bare}{\code{signature(object = "git_repository")}}
##'   \item{is.empty}{\code{signature(object = "git_repository")}}
##' }
##' @keywords methods
##' @export
setClass("git_repository",
         slots=c(path="character"),
         validity=function(object) {
             errors <- character()

             if(!identical(.Call("is_repository", object@path), TRUE))
                 errors <- c(errors, "Invalid repository")

             if (length(errors) == 0) TRUE else errors
         }
)

##' Coerce Git repository to a \code{data.frame}
##'
##' The commits in the repository are coerced to a \code{data.frame}
##'
##'
##' The \code{data.frame} have the following columns:
##' \describe{
##'
##'   \item{hex}{
##'     the SHA-1 hash as 40 characters of hexadecimal
##'   }
##'
##'   \item{summary}{
##'     the short "summary" of the git commit message.
##'   }
##'
##'   \item{message}{
##'     the full message of a commit
##'   }
##'
##'   \item{author}{
##'     full name of the author
##'   }
##'
##'   \item{email}{
##'     email of the author
##'   }
##'
##'   \item{when}{
##'     time when the commit happened
##'   }
##'
##' }
##' @name coerce-git_repository-method
##' @aliases coerce,git_repository,data.frame-method
##' @docType methods
##' @param from The repository \code{object}
##' @return \code{data.frame}
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Coerce commits to a data.frame
##' df <- as(repo, "data.frame")
##'
##' str(df)
##' }
setAs(from="git_repository",
      to="data.frame",
      def=function(from)
      {
          do.call("rbind", lapply(commits(from), function(x) {
              data.frame(hex              = x@hex,
                         summary          = x@summary,
                         message          = x@message,
                         author           = x@author@name,
                         email            = x@author@email,
                         when             = as(x@author@when, "POSIXct"),
                         stringsAsFactors = FALSE)
          }))
      }
)

##' Open a repository
##'
##' @param path A path to an existing local git repository
##' @return A S4 \code{git_repository} object
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Brief summary of repository
##' repo
##'
##' ## Summary of repository
##' summary(repo)
##'
##' ## Workdir of repository
##' workdir(repo)
##'
##' ## Check if repository is bare
##' is.bare(repo)
##'
##' ## Check if repository is empty
##' is.empty(repo)
##'
##' ## List all references in repository
##' references(repo)
##'
##' ## List all branches in repository
##' branches(repo)
##'
##' ## Get HEAD of repository
##' head(repo)
##'
##' ## Check if HEAD is head
##' is.head(head(repo))
##'
##' ## Check if HEAD is local
##' is.local(head(repo))
##'
##' ## List all tags in repository
##' tags(repo)
##' }
##'
repository <- function(path) {
    ## Argument checking
    stopifnot(is.character(path),
              identical(length(path), 1L),
              nchar(path) > 0)

    path <- normalizePath(path, winslash = "/", mustWork = TRUE)
    if(!file.info(path)$isdir)
        stop("path is not a directory")

    new("git_repository", path=path)
}

##' Init a repository
##'
##' @param path A path to where to init a git repository
##' @param bare If TRUE, a Git repository without a working directory
##' is created at the pointed path. If FALSE, provided path will be
##' considered as the working directory into which the .git directory
##' will be created.
##' @return A S4 \code{git_repository} object
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Init a repository
##' repo <- init("path/to/git2r")
##' }
init <- function(path, bare = FALSE) {
    ## Argument checking
    stopifnot(is.character(path),
              identical(length(path), 1L),
              nchar(path) > 0,
              is.logical(bare),
              identical(length(bare), 1L))

    path <- normalizePath(path, winslash = "/", mustWork = TRUE)
    if(!file.info(path)$isdir)
        stop("path is not a directory")

    .Call("init", path, bare)

    new("git_repository", path=path)
}

##' Clone a remote repository
##'
##' @param url the remote repository to clone
##' @param local_path local directory to clone to
##' @param progress show progress
##' @return A S4 \code{git_repository} object
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Clone a remote repository
##' repo <- clone("https://github.com/ropensci/git2r", "path/to/git2r")
##' }
clone <- function(url, local_path, progress = TRUE) {
    ## Argument checking
    stopifnot(is.character(url),
              is.character(local_path),
              is.logical(progress),
              identical(length(url), 1L),
              identical(length(local_path), 1L),
              identical(length(progress), 1L),
              nchar(url) > 0,
              nchar(local_path) > 0)

    .Call("clone", url, local_path, progress)

    repository(local_path)
}

##' Add file(s) to index
##'
##' @rdname add-methods
##' @docType methods
##' @param object The repository \code{object}.
##' @param path character vector with filenames to add. The path must
##' be relative to the repository's working filder.
##' @return invisible(NULL)
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Add file repository
##' add(repo, "file-to-add")
##' }
##'
setGeneric("add",
           signature = "object",
           function(object, path) standardGeneric("add"))

##' @rdname add-methods
##' @export
setMethod("add",
          signature(object = "git_repository"),
          function (object, path)
          {
              ## Argument checking
              stopifnot(is.character(path),
                        all(nchar(path) > 0))

              lapply(path, function(x) .Call("add", object, x))

              invisible(NULL)
          }
)

##' Commit
##'
##' @rdname commit-methods
##' @docType methods
##' @param object The repository \code{object}.
##' @param message The commit message.
##' @param reference Name of the reference that will be updated to
##' point to this commit.
##' @param author Signature with author and author time of commit.
##' @param committer Signature with committer and commit time of commit.
##' @return \code{git_commit} object
##' @keywords methods
setGeneric("commit",
           signature = "object",
           function(object,
                    message = NULL,
                    reference = "HEAD",
                    author = default_signature(object),
                    committer = default_signature(object))
           standardGeneric("commit"))

##' @rdname commit-methods
##' @export
setMethod("commit",
          signature(object = "git_repository"),
          function (object,
                    message,
                    reference,
                    author,
                    committer)
          {
              ## Argument checking
              stopifnot(is.character(message),
                        identical(length(message), 1L),
                        nchar(message[1]) > 0,
                        is(author, "git_signature"),
                        is(committer, "git_signature"))

              parents <- character(0)
              if(!is.empty(object)) {
                  parents <- c(parents, head(object)@hex)
              }

              .Call("commit", object, message, author, committer, parents)
          }
)

##' Get HEAD for a repo
##'
##' @rdname head-methods
##' @docType methods
##' @param x The repository \code{x} to check head
##' @return Character vector with head
##' @keywords methods
##' @export
setMethod("head",
          signature(x = "git_repository"),
          function (x)
          {
              b <- branches(x)

              if(length(b)) {
                  b <- b[sapply(b, is.head)]
                  if(identical(length(b), 1L)) {
                      return(b[[1]])
                  }

                  return(b)
              }

              return(NULL)
          }
)

##' Check if repository is bare
##'
##' @rdname is.bare-methods
##' @docType methods
##' @param object The \code{object} to check if it's a bare repository
##' @return TRUE if bare repository, else FALSE
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Check if it's a bare repository
##' is.bare(repo)
##' }
##'
setGeneric("is.bare",
           signature = "object",
           function(object) standardGeneric("is.bare"))

##' @rdname is.bare-methods
##' @export
setMethod("is.bare",
          signature(object = "git_repository"),
          function (object)
          {
              .Call("is_bare", object)
          }
)

##' Check if HEAD of repository is detached
##'
##' @rdname is.detached-methods
##' @docType methods
##' @param object The repository \code{object}
##' @return TRUE if repository HEAD is detached, else FALSE
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Check if repository HEAD is detached
##' is.detached(repo)
##' }
##'
setGeneric("is.detached",
           signature = "object",
           function(object)
           standardGeneric("is.detached"))

##' @rdname is.detached-methods
##' @export
setMethod("is.detached",
          signature(object = "git_repository"),
          function (object)
          {
              .Call("is_detached", object)
          }
)

##' Check if repository is empty
##'
##' @rdname is.empty-methods
##' @docType methods
##' @param object The \code{object} to check if it's a empty repository
##' @return TRUE or FALSE
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Check if it's an empty repository
##' is.empty(repo)
##' }
##'
setGeneric("is.empty",
           signature = "object",
           function(object) standardGeneric("is.empty"))

##' @rdname is.empty-methods
##' @export
setMethod("is.empty",
          signature(object = "git_repository"),
          function (object)
          {
              .Call("is_empty", object)
          }
)

##' Lookup
##'
##' Lookup one object in a repository.
##' @rdname lookup-methods
##' @docType methods
##' @param object The repository \code{object}.
##' @param hex the identity of the object to lookup. Must be 4 to 40
##' characters long.
##' @return a \code{git_blob} or \code{git_commit} or \code{git_tag}
##' or \code{git_tree} object
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Lookup commit in repository
##' lookup(repo, "6fd0b22")
##' }
##'
setGeneric("lookup",
           signature = "object",
           function(object, hex)
           standardGeneric("lookup"))

##' @rdname lookup-methods
##' @export
setMethod("lookup",
          signature(object = "git_repository"),
          function (object, hex)
          {
              .Call("lookup", object, hex)
          }
)

##' Get the configured remotes for a repo
##'
##' @rdname remotes-methods
##' @docType methods
##' @param object The repository \code{object} to check remotes
##' @return Character vector with remotes
##' @keywords methods
setGeneric("remotes",
           signature = "object",
           function(object) standardGeneric("remotes"))

##' @rdname remotes-methods
##' @export
setMethod("remotes",
          signature(object = "git_repository"),
          function (object)
          {
              .Call("remotes", object)
          }
)

##' Get the remote url for remotes in a repo
##'
##' @rdname remote_url-methods
##' @docType methods
##' @param object The repository \code{object} to check remote_url
##' @param remote :TODO:DOCUMENTATION:
##' @return Character vector with remote_url
##' @keywords methods
setGeneric("remote_url",
           signature = "object",
           function(object, remote = remotes(object)) standardGeneric("remote_url"))

##' @rdname remote_url-methods
##' @export
setMethod("remote_url",
          signature(object = "git_repository"),
          function (object, remote)
          {
              .Call("remote_url", object, remote)
          }
)

##' Get the signature
##'
##' Get the signature according to the repository's configuration
##' @rdname default_signature-methods
##' @docType methods
##' @param object The repository \code{object} to check signature
##' @return Character vector with signature
##' @keywords methods
setGeneric("default_signature",
           signature = "object",
           function(object) standardGeneric("default_signature"))

##' @rdname default_signature-methods
##' @export
setMethod("default_signature",
          signature(object = "git_repository"),
          function (object)
          {
              .Call("default_signature", object)
          }
)

##' Brief summary of repository
##'
##' @aliases show,git_repository-methods
##' @docType methods
##' @param object The repository \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
setMethod("show",
          signature(object = "git_repository"),
          function(object)
          {
              lapply(remotes(object), function(remote) {
                  cat(sprintf("Remote:   @ %s (%s)\n",
                              remote,
                              remote_url(object, remote)))
              })

              if(is.empty(object)) {
                  cat(sprintf("Local:    %s\n", workdir(object)))
                  cat("Head:     nothing commited (yet)\n")
              } else if(is.detached(object)) {
                  cat(sprintf("Local:    (detached) %s\n", workdir(object)))
              } else {
                  cat(sprintf("Local:    %s %s\n",
                              head(object)@shorthand,
                              workdir(object)))
              }
          }
)

##' Summary of repository
##'
##' @aliases summary,git_repository-methods
##' @docType methods
##' @param object The repository \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
setMethod("summary",
          signature(object = "git_repository"),
          function(object, ...)
          {
              show(object)
              cat("\n")

              n_branches <- sum(!is.na(unique(sapply(branches(object), slot, "hex"))))
              n_tags <- sum(!is.na(unique(sapply(tags(object), slot, "hex"))))

              work <- commits(object)
              n_commits <- length(work)
              n_authors <- length(unique(sapply(lapply(work, slot, "author"), slot, "name")))

              s <- .Call("status", object, TRUE, TRUE, TRUE, TRUE)
              n_ignored <- length(s$ignored)
              n_untracked <- length(s$untracked)
              n_unstaged <- length(s$unstaged)
              n_staged <- length(s$staged)

              ## Determine max characters needed to display numbers
              n <- max(sapply(c(n_branches, n_tags, n_commits, n_authors,
                                n_ignored, n_untracked, n_unstaged, n_staged),
                              nchar))

              fmt <- paste0("Branches:        %", n, "i\n",
                            "Tags:            %", n, "i\n",
                            "Commits:         %", n, "i\n",
                            "Contributors:    %", n, "i\n",
                            "Ignored files:   %", n, "i\n",
                            "Untracked files: %", n, "i\n",
                            "Unstaged files:  %", n, "i\n",
                            "Staged files:    %", n, "i\n")
              cat(sprintf(fmt, n_branches, n_tags, n_commits, n_authors,
                          n_ignored, n_untracked, n_unstaged, n_staged))
          }
)

##' Workdir of repository
##'
##' @rdname workdir-methods
##' @docType methods
##' @param object The repository \code{object} to check workdir
##' @return Character vector with workdir. If the repository is bare,
##' \code{NULL} will be returned.
##' @keywords methods
setGeneric("workdir",
           signature = "object",
           function(object) standardGeneric("workdir"))

##' @rdname workdir-methods
##' @export
setMethod("workdir",
          signature(object = "git_repository"),
          function (object)
          {
              .Call("workdir", object)
          }
)
