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
##'   \item{is_bare}{\code{signature(object = "git_repository")}}
##'   \item{is_empty}{\code{signature(object = "git_repository")}}
##' }
##' @keywords methods
##' @export
setClass("git_repository",
         slots=c(path="character"),
         validity=function(object) {
             errors <- character()

             can_open <- .Call("git2r_repository_can_open", object@path)
             if(!identical(can_open, TRUE))
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
##' @param discover Discover repository from path. Default is FALSE.
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
##' is_bare(repo)
##'
##' ## Check if repository is empty
##' is_empty(repo)
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
##' is_head(head(repo))
##'
##' ## Check if HEAD is local
##' is_local(head(repo))
##'
##' ## List all tags in repository
##' tags(repo)
##' }
##'
repository <- function(path, discover = FALSE) {
    ## Argument checking
    stopifnot(is.character(path),
              identical(length(path), 1L),
              nchar(path) > 0)

    if (discover) {
        path <- discover_repository(path)
    } else {
        path <- normalizePath(path, winslash = "/", mustWork = TRUE)
        if(!file.info(path)$isdir)
            stop("path is not a directory")
    }

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

    .Call("git2r_repository_init", path, bare)

    new("git_repository", path=path)
}

##' Clone a remote repository
##'
##' @param url The remote repository to clone
##' @param local_path Local directory to clone to
##' @param credentials The credentials for remote repository access. Default
##' is NULL.
##' @param progress Show progress. Default is TRUE.
##' @return A S4 \code{git_repository} object
##' @seealso \code{\linkS4class{cred_plaintext}},
##' \code{\linkS4class{cred_ssh_key}}
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Clone a remote repository
##' repo <- clone("https://github.com/ropensci/git2r", "path/to/git2r")
##' }
clone <- function(url, local_path, credentials = NULL, progress = TRUE) {
    ## Argument checking
    stopifnot(is.character(url),
              is.character(local_path),
              is.logical(progress),
              identical(length(url), 1L),
              identical(length(local_path), 1L),
              identical(length(progress), 1L),
              nchar(url) > 0,
              nchar(local_path) > 0)

    .Call("git2r_clone", url, local_path, credentials, progress)

    repository(local_path)
}

##' Commit
##'
##' @rdname commit-methods
##' @docType methods
##' @param repo The repository \code{object}.
##' @param message The commit message.
##' @param reference Name of the reference that will be updated to
##' point to this commit.
##' @param author Signature with author and author time of commit.
##' @param committer Signature with committer and commit time of commit.
##' @return \code{git_commit} object
##' @keywords methods
setGeneric("commit",
           signature = "repo",
           function(repo,
                    message = NULL,
                    reference = "HEAD",
                    author = default_signature(repo),
                    committer = default_signature(repo))
           standardGeneric("commit"))

##' @rdname commit-methods
##' @export
setMethod("commit",
          signature(repo = "git_repository"),
          function (repo,
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
              if(!is_empty(repo)) {
                  parents <- c(parents, branch_target(head(repo)))
              }

              .Call("git2r_commit_create",
                    repo,
                    message,
                    author,
                    committer,
                    parents)
          }
)

##' Get HEAD for a repository
##'
##' @rdname head-methods
##' @docType methods
##' @param x The repository \code{x} to check head
##' @return NULL if unborn branch or not found. S4 class git_branch if
##' not a detached head. S4 class git_commit if detached head
##' @keywords methods
##' @export
setMethod("head",
          signature(x = "git_repository"),
          function (x)
          {
              .Call("git2r_repository_head", x)
          }
)

##' Check if repository is bare
##'
##' @rdname is_bare-methods
##' @docType methods
##' @param repo The repository to check if it's bare
##' @return TRUE if bare repository, else FALSE
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Check if it's a bare repository
##' is_bare(repo)
##' }
##'
setGeneric("is_bare",
           signature = "repo",
           function(repo)
           standardGeneric("is_bare"))

##' @rdname is_bare-methods
##' @export
setMethod("is_bare",
          signature(repo = "git_repository"),
          function (repo)
          {
              .Call("git2r_repository_is_bare", repo)
          }
)

##' Check if HEAD of repository is detached
##'
##' @rdname is_detached-methods
##' @docType methods
##' @param repo The repository \code{object}
##' @return TRUE if repository HEAD is detached, else FALSE
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Check if repository HEAD is detached
##' is_detached(repo)
##' }
##'
setGeneric("is_detached",
           signature = "repo",
           function(repo)
           standardGeneric("is_detached"))

##' @rdname is_detached-methods
##' @export
setMethod("is_detached",
          signature(repo = "git_repository"),
          function (repo)
          {
              .Call("git2r_repository_head_detached", repo)
          }
)

##' Check if repository is empty
##'
##' @rdname is_empty-methods
##' @docType methods
##' @param repo The repository to check if it's empty
##' @return TRUE if empty else FALSE
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Check if it's an empty repository
##' is_empty(repo)
##' }
##'
setGeneric("is_empty",
           signature = "repo",
           function(repo)
           standardGeneric("is_empty"))

##' @rdname is_empty-methods
##' @export
setMethod("is_empty",
          signature(repo = "git_repository"),
          function (repo)
          {
              .Call("git2r_repository_is_empty", repo)
          }
)

##' Determine if the repository was a shallow clone
##'
##' @rdname is_shallow-methods
##' @docType methods
##' @param repo The repository
##' @return TRUE if shallow clone, else FALSE
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Check if it's a shallow clone
##' is_shallow(repo)
##' }
##'
setGeneric("is_shallow",
           signature = "repo",
           function(repo)
           standardGeneric("is_shallow"))

##' @rdname is_shallow-methods
##' @export
setMethod("is_shallow",
          signature(repo = "git_repository"),
          function (repo)
          {
              .Call("git2r_repository_is_shallow", repo)
          }
)

##' Lookup
##'
##' Lookup one object in a repository.
##' @rdname lookup-methods
##' @docType methods
##' @param repo The repository.
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
           signature = c("repo", "hex"),
           function(repo, hex)
           standardGeneric("lookup"))

##' @rdname lookup-methods
##' @export
setMethod("lookup",
          signature(repo = "git_repository",
                    hex  = "character"),
          function (repo, hex)
          {
              .Call("git2r_object_lookup", repo, hex)
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
           function(object)
           standardGeneric("remotes"))

##' @rdname remotes-methods
##' @export
setMethod("remotes",
          signature(object = "git_repository"),
          function (object)
          {
              .Call("git2r_remote_list", object)
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
           function(object, remote = remotes(object))
           standardGeneric("remote_url"))

##' @rdname remote_url-methods
##' @export
setMethod("remote_url",
          signature(object = "git_repository"),
          function (object, remote)
          {
              .Call("git2r_remote_url", object, remote)
          }
)

##' Add a remote to a repo
##'
##' @rdname remote_add-methods
##' @docType methods
##' @param repo The repository \code{object} to add the remote to
##' @param name Short name of the remote repository
##' @param url URL of the remote repository
##' @return NULL, invisibly
##' @keywords methods
setGeneric("remote_add",
           signature = c("repo", "name", "url"),
           function(repo, name, url)
           standardGeneric("remote_add"))

##' @rdname remote_add-methods
##' @export
setMethod("remote_add",
          signature(repo = "git_repository",
                    name = "character",
                    url  = "character"),
          function(repo, name, url)
          {
              ret <- .Call("git2r_remote_add", repo, name, url)
              invisible(ret)
          }
)

##' Rename a remote
##'
##' @rdname remote_rename-methods
##' @docType methods
##' @param repo The repository in which the remote should be renamed.
##' @param oldname Old name of the remote
##' @param newname New name of the remote
##' @return NULL, invisibly
##' @keywords methods
setGeneric("remote_rename",
           signature = c("repo", "oldname", "newname"),
           function(repo, oldname, newname)
           standardGeneric("remote_rename"))

##' @rdname remote_rename-methods
##' @export
setMethod("remote_rename",
          signature(repo    = "git_repository",
                    oldname = "character",
                    newname = "character"),
          function(repo, oldname, newname)
          {
              ret <- .Call("git2r_remote_rename", repo, oldname, newname)
              invisible(ret)
          }
)

##' Remove a remote
##'
##' @rdname remote_remove-methods
##' @docType methods
##' @param repo The repository to work on
##' @param name The name of the remote to remove
##' @return NULL, invisibly
##' @keywords methods
setGeneric("remote_remove",
           signature = c("repo", "name"),
           function(repo, name)
           standardGeneric("remote_remove"))

##' @rdname remote_remove-methods
##' @export
setMethod("remote_remove",
          signature(repo = "git_repository",
                    name = "character"),
          function(repo, name)
          {
              ret <- .Call("git2r_remote_remove", repo, name)
              invisible(ret)
          }
)

##' Get the signature
##'
##' Get the signature according to the repository's configuration
##' @rdname default_signature-methods
##' @docType methods
##' @param repo The repository \code{object} to check signature
##' @return S4 class git_signature
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Get the default signature
##' default_signature(repo)
##' }
setGeneric("default_signature",
           signature = "repo",
           function(repo)
           standardGeneric("default_signature"))

##' @rdname default_signature-methods
##' @export
setMethod("default_signature",
          signature(repo = "git_repository"),
          function (repo)
          {
              .Call("git2r_signature_default", repo)
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

              if(is_empty(object)) {
                  cat(sprintf("Local:    %s\n", workdir(object)))
                  cat("Head:     nothing commited (yet)\n")
              } else if(is_detached(object)) {
                  cat(sprintf("Local:    (detached) %s\n", workdir(object)))
              } else {
                  cat(sprintf("Local:    %s %s\n",
                              head(object)@name,
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

              n_branches <- sum(!is.na(unique(sapply(branches(object), branch_target))))
              n_tags <- sum(!is.na(unique(sapply(tags(object), slot, "hex"))))

              work <- commits(object)
              n_commits <- length(work)
              n_authors <- length(unique(sapply(lapply(work, slot, "author"), slot, "name")))

              s <- .Call("git2r_status_list", object, TRUE, TRUE, TRUE, TRUE)
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
##' @param repo The repository \code{object}.
##' @return Character vector with the path of the workdir. If the
##' repository is bare, \code{NULL} will be returned.
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Get the path of the workdir for repository
##' workdir(repo)
##' }
setGeneric("workdir",
           signature = "repo",
           function(repo)
           standardGeneric("workdir"))

##' @rdname workdir-methods
##' @export
setMethod("workdir",
          signature(repo = "git_repository"),
          function (repo)
          {
              .Call("git2r_repository_workdir", repo)
          }
)

##' Find path to repository for any file
##'
##' libgit's git_discover_repository is used to identify the location of the
##'   repository. The path will therefore be terminated by a file separator.
##' @rdname discover_repository-methods
##' @docType methods
##' @param path A character vector specifying the path to a file or folder
##' @return Character vector with path to repository or NULL if this cannot be
##'   established.
##' @keywords methods
##' @examples
##' \dontrun{
##' path='/path/to/my/new/repo'
##' init(path)
##' discover_repository(path)
##' # /path/to/my/new/repo/.git/
##' }
setGeneric("discover_repository",
           signature = "path",
           function(path)
           standardGeneric("discover_repository"))

##' @rdname discover_repository-methods
##' @export
setMethod("discover_repository",
          signature(path = "character"),
          function (path)
          {
              .Call("git2r_repository_discover", path)
          }
)
