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

##' Pull
##'
##' @template repo-param
##' @param credentials The credentials for remote repository
##'     access. Default is NULL. To use and query an ssh-agent for the
##'     ssh key credentials, let this parameter be NULL (the default).
##' @param merger Who made the merge, if the merge is non-fast forward
##'     merge that creates a merge commit. The
##'     \code{default_signature} for \code{repo} is used if this
##'     parameter is \code{NULL}.
##' @template return-git_merge_result
##' @export
##' @examples
##' \dontrun{
##' ## Initialize repositories
##' path_bare <- tempfile(pattern="git2r-")
##' path_repo_1 <- tempfile(pattern="git2r-")
##' path_repo_2 <- tempfile(pattern="git2r-")
##' dir.create(path_bare)
##' dir.create(path_repo_1)
##' dir.create(path_repo_2)
##' repo_bare <- init(path_bare, bare = TRUE)
##' repo_1 <- clone(path_bare, path_repo_1)
##'
##' ## Config first user and commit a file
##' config(repo_1, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Write to a file and commit
##' lines <- "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do"
##' writeLines(lines, file.path(path_repo_1, "example.txt"))
##' add(repo_1, "example.txt")
##' commit(repo_1, "First commit message")
##'
##' ## Push commits from first repository to bare repository
##' ## Adds an upstream tracking branch to branch 'master'
##' push(repo_1, "origin", "refs/heads/master")
##'
##' ## Clone to second repository
##' repo_2 <- clone(path_bare, path_repo_2)
##' config(repo_2, user.name = "Bob", user.email = "bob@@example.org")
##'
##' ## Change file and commit
##' lines <- c(
##'   "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'   "eiusmod tempor incididunt ut labore et dolore magna aliqua.")
##' writeLines(lines, file.path(path_repo_1, "example.txt"))
##' add(repo_1, "example.txt")
##' commit(repo_1, "Second commit message")
##'
##' ## Push commits from first repository to bare repository
##' push(repo_1)
##'
##' ## Pull changes to repo_2
##' pull(repo_2)
##'
##' ## Change file again and commit. This time in repository 2
##' lines <- c(
##'   "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'   "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad",
##'   "minim veniam, quis nostrud exercitation ullamco laboris nisi ut")
##' writeLines(lines, file.path(path_repo_2, "example.txt"))
##' add(repo_2, "example.txt")
##' commit(repo_2, "Third commit message")
##'
##' ## Push commits from second repository to bare repository
##' push(repo_2)
##'
##' ## Pull changes to repo_1
##' pull(repo_1)
##'
##' ## List commits in repositories
##' commits(repo_1)
##' commits(repo_2)
##' commits(repo_bare)
##' }
pull <- function(repo = ".", credentials = NULL, merger = NULL) {
    repo <- lookup_repository(repo)
    if (is.null(merger))
        merger <- default_signature(repo)
    current_branch <- repository_head(repo)

    if (is.null(current_branch))
        stop("'branch' is NULL")
    if (!is_local(current_branch))
        stop("'branch' is not local")
    upstream_branch <- branch_get_upstream(current_branch)
    if (is.null(upstream_branch))
        stop("'branch' is not tracking a remote branch")

    fetch(repo        = repo,
          name        = branch_remote_name(upstream_branch),
          credentials = credentials)

    ## fetch heads marked for merge
    fh <- fetch_heads(repo)
    fh <- fh[vapply(fh, "[[", logical(1), "is_merge")]

    if (identical(length(fh), 0L))
        stop("Remote ref was not fetched")

    .Call(git2r_merge_fetch_heads, fh, merger)
}
