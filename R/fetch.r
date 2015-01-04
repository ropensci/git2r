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

##' Fetch new data and update tips
##'
##' @rdname fetch-methods
##' @docType methods
##' @param repo the repository
##' @param name the remote's name
##' @param credentials The credentials for remote repository
##' access. Default is NULL.
##' @return invisible \code{\linkS4class{git_transfer_progress}} object
##' @keywords methods
##' @include S4_classes.r
##' @examples
##' \dontrun{
##' ## Initialize three temporary repositories
##' path_bare <- tempfile(pattern="git2r-")
##' path_repo_1 <- tempfile(pattern="git2r-")
##' path_repo_2 <- tempfile(pattern="git2r-")
##'
##' dir.create(path_bare)
##' dir.create(path_repo_1)
##' dir.create(path_repo_2)
##'
##' bare_repo <- init(path_bare, bare = TRUE)
##' repo_1 <- clone(path_bare, path_repo_1)
##' repo_2 <- clone(path_bare, path_repo_2)
##'
##' config(repo_1, user.name="Repo One", user.email="repo.one@@example.org")
##' config(repo_2, user.name="Repo Two", user.email="repo.two@@example.org")
##'
##' ## Add changes to repo 1
##' writeLines("Lorem ipsum dolor sit amet",
##'            con = file.path(path_repo_1, "example.txt"))
##' add(repo_1, "example.txt")
##' commit(repo_1, "Commit message")
##'
##' ## Push changes from repo 1 to origin (bare_repo)
##' push(repo_1, "origin", "refs/heads/master")
##'
##' ## Fetch changes from origin (bare_repo) to repo 2
##' fetch(repo_2, "origin")
##'
##' ## List updated heads
##' fetch_heads(repo_2)
##' }
setGeneric("fetch",
           signature = "repo",
           function(repo,
                    name,
                    credentials = NULL) standardGeneric("fetch"))

##' @rdname fetch-methods
##' @export
setMethod("fetch",
          signature(repo = "git_repository"),
          function (repo,
                    name,
                    credentials)
          {
              result <- .Call(
                  git2r_remote_fetch,
                  repo,
                  name,
                  credentials,
                  "fetch",
                  default_signature(repo))

              invisible(result)
          }
)

##' Get updated heads during the last fetch.
##'
##' @rdname fetch_heads-methods
##' @docType methods
##' @param repo the repository
##' @return list with the S4 class \code{\linkS4class{git_fetch_head}}
##' entries. NULL if there is no FETCH_HEAD file.
##' @keywords methods
##' @include S4_classes.r
##' @examples
##' \dontrun{
##' ## Initialize three temporary repositories
##' path_bare <- tempfile(pattern="git2r-")
##' path_repo_1 <- tempfile(pattern="git2r-")
##' path_repo_2 <- tempfile(pattern="git2r-")
##'
##' dir.create(path_bare)
##' dir.create(path_repo_1)
##' dir.create(path_repo_2)
##'
##' bare_repo <- init(path_bare, bare = TRUE)
##' repo_1 <- clone(path_bare, path_repo_1)
##' repo_2 <- clone(path_bare, path_repo_2)
##'
##' config(repo_1, user.name="Repo One", user.email="repo.one@@example.org")
##' config(repo_2, user.name="Repo Two", user.email="repo.two@@example.org")
##'
##' ## Add changes to repo 1
##' writeLines("Lorem ipsum dolor sit amet",
##'            con = file.path(path_repo_1, "example.txt"))
##' add(repo_1, "example.txt")
##' commit(repo_1, "Commit message")
##'
##' ## Push changes from repo 1 to origin (bare_repo)
##' push(repo_1, "origin", "refs/heads/master")
##'
##' ## Fetch changes from origin (bare_repo) to repo 2
##' fetch(repo_2, "origin")
##'
##' ## List updated heads
##' fetch_heads(repo_2)
##' }
setGeneric("fetch_heads",
           signature = "repo",
           function(repo)
           standardGeneric("fetch_heads"))

##' @rdname fetch_heads-methods
##' @export
setMethod("fetch_heads",
          signature(repo = "git_repository"),
          function (repo)
          {
              .Call(git2r_repository_fetch_heads, repo)
          }
)
