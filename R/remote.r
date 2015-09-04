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

##' Get the configured remotes for a repo
##'
##' @rdname remotes-methods
##' @docType methods
##' @param repo The repository \code{object}
##' \code{\linkS4class{git_repository}}. If the \code{repo} argument
##' is missing, the repository is searched for with
##' \code{\link{discover_repository}} in the current working
##' directory.
##' @return Character vector with remotes
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user and commit a file
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Add a remote
##' remote_add(repo, "playground", "https://example.org/git2r/playground")
##' remotes(repo)
##' remote_url(repo, "playground")
##'
##' ## Rename a remote
##' remote_rename(repo, "playground", "foobar")
##' remotes(repo)
##' remote_url(repo, "foobar")
##'
##' ## Remove a remote
##' remote_remove(repo, "foobar")
##' remotes(repo)
##' }
setGeneric("remotes",
           signature = "repo",
           function(repo)
           standardGeneric("remotes"))

##' @rdname remotes-methods
##' @export
setMethod("remotes",
          signature(repo = "missing"),
          function()
          {
              callGeneric(repo = lookup_repository())
          }
)

##' @rdname remotes-methods
##' @export
setMethod("remotes",
          signature(repo = "git_repository"),
          function(repo)
          {
              .Call(git2r_remote_list, repo)
          }
)

##' Add a remote to a repo
##'
##' @rdname remote_add-methods
##' @docType methods
##' @param repo The repository to add the remote to
##' @param name Short name of the remote repository
##' @param url URL of the remote repository
##' @return NULL, invisibly
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user and commit a file
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Add a remote
##' remote_add(repo, "playground", "https://example.org/git2r/playground")
##' remotes(repo)
##' remote_url(repo, "playground")
##'
##' ## Rename a remote
##' remote_rename(repo, "playground", "foobar")
##' remotes(repo)
##' remote_url(repo, "foobar")
##'
##' ## Remove a remote
##' remote_remove(repo, "foobar")
##' remotes(repo)
##' }
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
              ret <- .Call(git2r_remote_add, repo, name, url)
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
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user and commit a file
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Add a remote
##' remote_add(repo, "playground", "https://example.org/git2r/playground")
##' remotes(repo)
##' remote_url(repo, "playground")
##'
##' ## Rename a remote
##' remote_rename(repo, "playground", "foobar")
##' remotes(repo)
##' remote_url(repo, "foobar")
##'
##' ## Remove a remote
##' remote_remove(repo, "foobar")
##' remotes(repo)
##' }
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
              ret <- .Call(git2r_remote_rename, repo, oldname, newname)
              invisible(ret)
          }
)

##' Remove a remote
##'
##' All remote-tracking branches and configuration settings for the
##' remote will be removed.
##' @rdname remote_remove-methods
##' @docType methods
##' @param repo The repository to work on
##' @param name The name of the remote to remove
##' @return NULL, invisibly
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user and commit a file
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Add a remote
##' remote_add(repo, "playground", "https://example.org/git2r/playground")
##' remotes(repo)
##' remote_url(repo, "playground")
##'
##' ## Rename a remote
##' remote_rename(repo, "playground", "foobar")
##' remotes(repo)
##' remote_url(repo, "foobar")
##'
##' ## Remove a remote
##' remote_remove(repo, "foobar")
##' remotes(repo)
##' }
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
              ret <- .Call(git2r_remote_remove, repo, name)
              invisible(ret)
          }
)

##' Set the remote's url in the configuration
##'
##' This assumes the common case of a single-url remote and will
##' otherwise raise an error.
##' @rdname remote_set_url-methods
##' @docType methods
##' @param repo The repository in which to perform the change
##' @param name The name of the remote
##' @param url The \code{url} to set
##' @return NULL, invisibly
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user and commit a file
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Add a remote
##' remote_add(repo, "playground", "https://example.org/git2r/playground")
##' remotes(repo)
##' remote_url(repo, "playground")
##'
##' ## Rename a remote
##' remote_rename(repo, "playground", "foobar")
##' remotes(repo)
##' remote_url(repo, "foobar")
##'
##' ## Set remote url
##' remote_set_url(repo, "foobar", "https://example.org/git2r/foobar")
##' remotes(repo)
##' remote_url(repo, "foobar")
##'
##' ## Remove a remote
##' remote_remove(repo, "foobar")
##' remotes(repo)
##' }
setGeneric("remote_set_url",
           signature = c("repo", "name", "url"),
           function(repo, name, url)
           standardGeneric("remote_set_url"))

##' @rdname remote_set_url-methods
##' @export
setMethod("remote_set_url",
          signature(repo = "git_repository",
                    name = "character",
                    url  = "character"),
          function(repo, name, url)
          {
              ret <- .Call(git2r_remote_set_url, repo, name, url)
              invisible(ret)
          }
)

##' Get the remote url for remotes in a repo
##'
##' @rdname remote_url-methods
##' @docType methods
##' @param repo The repository to get remote urls from
##' @param remote Character vector with the remotes to get the url
##' from. Default is the remotes of the repository.
##' @return Character vector with remote_url for each of the remote
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user and commit a file
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Add a remote
##' remote_add(repo, "playground", "https://example.org/git2r/playground")
##' remotes(repo)
##' remote_url(repo, "playground")
##'
##' ## Rename a remote
##' remote_rename(repo, "playground", "foobar")
##' remotes(repo)
##' remote_url(repo, "foobar")
##'
##' ## Remove a remote
##' remote_remove(repo, "foobar")
##' remotes(repo)
##' }
setGeneric("remote_url",
           signature = "repo",
           function(repo, remote = remotes(repo))
           standardGeneric("remote_url"))

##' @rdname remote_url-methods
##' @export
setMethod("remote_url",
          signature(repo = "git_repository"),
          function(repo, remote)
          {
              .Call(git2r_remote_url, repo, remote)
          }
)


##' List references in a remote repository
##'
##' Displays references available in a remote repository along with the associated commit IDs.
##' @rdname ls_remote-methods
##' @docType methods
##' @param name Character vector with the with "remote" repository URL to query.
##' @return Character vector for each reference with the associated commit IDs.
##' @param repo an optional repository object used to store data temporarily.
##' @keywords methods
##' @examples
##' \dontrun{
##' ls_remote("https://github.com/ropensci/git2r")
##' }
##' @export
setGeneric("ls_remote",
           signature = c("name", "repo"),
           function(name, repo)
           standardGeneric("ls_remote"))

##' @rdname ls_remote-methods
##' @export
setMethod("ls_remote",
          signature(name = "character",
                    repo = "missing"),
          function(name)
          {
              .Call(git2r_ls_remote, git2r::init(tempdir()), name)
          }
)
##' @rdname ls_remote-methods
##' @export
setMethod("ls_remote",
          signature(name = "character",
                    repo = "git_repository"),
          function(name, repo)
          {
              .Call(git2r_ls_remote, repo, name)
          }
)
