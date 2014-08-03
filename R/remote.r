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

##' Get the configured remotes for a repo
##'
##' @rdname remotes-methods
##' @docType methods
##' @param repo The repository to get remotes from
##' @return Character vector with remotes
##' @keywords methods
setGeneric("remotes",
           signature = "repo",
           function(repo)
           standardGeneric("remotes"))

##' @rdname remotes-methods
##' @export
setMethod("remotes",
          signature(repo = "git_repository"),
          function (repo)
          {
              .Call("git2r_remote_list", repo)
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
##' All remote-tracking branches and configuration settings for the
##' remote will be removed.
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

##' Get the remote url for remotes in a repo
##'
##' @rdname remote_url-methods
##' @docType methods
##' @param repo The repository to get remote urls from
##' @param remote Character vector with the remotes to get the url
##' from. Default is the remotes of the repository.
##' @return Character vector with remote_url for each of the remote
##' @keywords methods
##' @examples \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Get the url's for the remotes of the repository
##' remote_url(repo)
##'
##' ## Get the url for a specific remote of the repository
##' remote_url(repo, "origin")
##' }
setGeneric("remote_url",
           signature = "repo",
           function(repo, remote = remotes(repo))
           standardGeneric("remote_url"))

##' @rdname remote_url-methods
##' @export
setMethod("remote_url",
          signature(repo = "git_repository"),
          function (repo, remote)
          {
              .Call("git2r_remote_url", repo, remote)
          }
)
