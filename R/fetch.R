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

##' Fetch new data and update tips
##'
##' @template repo-param
##' @param name the remote's name
##' @param credentials The credentials for remote repository
##'     access. Default is NULL. To use and query an ssh-agent for the
##'     ssh key credentials, let this parameter be NULL (the default).
##' @param verbose Print information each time a reference is updated
##'     locally. Default is \code{TRUE}.
##' @param refspec The refs to fetch and which local refs to update,
##'     see examples. Pass NULL to use the
##'     \code{remote.<repository>.fetch} variable. Default is
##'     \code{NULL}.
##' @return invisible list of class \code{git_transfer_progress}
##'     with statistics from the fetch operation:
##' \describe{
##'   \item{total_objects}{
##'     Number of objects in the packfile being downloaded
##'   }
##'   \item{indexed_objects}{
##'     Received objects that have been hashed
##'   }
##'   \item{received_objects}{
##'     Objects which have been downloaded
##'   }
##'   \item{total_deltas}{
##'     Total number of deltas in the pack
##'   }
##'   \item{indexed_deltas}{
##'     Deltas which have been indexed
##'   }
##'   \item{local_objects}{
##'     Locally-available objects that have been injected in order to
##'     fix a thin pack
##'   }
##'   \item{received_bytes}{
##'     Size of the packfile received up to now
##'   }
##' }
##' @export
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
##' config(repo_1, user.name = "Alice", user.email = "alice@@example.org")
##' config(repo_2, user.name = "Bob", user.email = "bob@@example.org")
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
##'
##' ## Checking out GitHub pull requests locally
##' path <- tempfile(pattern="ghit-")
##' repo <- clone("https://github.com/leeper/ghit", path)
##' fetch(repo, "origin", refspec = "pull/13/head:refs/heads/BRANCHNAME")
##' checkout(repo, "BRANCHNAME")
##' summary(repo)
##' }
fetch <- function(repo = ".", name = NULL, credentials = NULL,
                  verbose = TRUE, refspec = NULL) {
    invisible(.Call(git2r_remote_fetch, lookup_repository(repo),
                    name, credentials, "fetch", verbose, refspec))
}

##' Get updated heads during the last fetch.
##'
##' @template repo-param
##' @return list with \code{git_fetch_head} entries. NULL if there is
##'     no FETCH_HEAD file.
##' @export
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
##' config(repo_1, user.name = "Alice", user.email = "alice@@example.org")
##' config(repo_2, user.name = "Bob", user.email = "bob@@example.org")
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
fetch_heads <- function(repo = ".")  {
    .Call(git2r_repository_fetch_heads, lookup_repository(repo))
}
