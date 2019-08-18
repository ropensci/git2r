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

##' List and view reflog information
##'
##' @template repo-param
##' @param refname The name of the reference to list. 'HEAD' by
##'     default.
##' @return S3 class \code{git_reflog} with git_reflog_entry objects.
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Config user
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Write to a file and commit
##' lines <- "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do"
##' writeLines(lines, file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Change file and commit
##' lines <- c(
##'   "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'   "eiusmod tempor incididunt ut labore et dolore magna aliqua.")
##' writeLines(lines, file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "Second commit message")
##'
##' ## Change file again and commit
##' lines <- c(
##'   "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'   "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad",
##'   "minim veniam, quis nostrud exercitation ullamco laboris nisi ut")
##' writeLines(lines, file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "Third commit message")
##'
##' ## View reflog
##' reflog(repo)
##' }
reflog <- function(repo = ".", refname = "HEAD") {
    structure(.Call(git2r_reflog_list, lookup_repository(repo), refname),
              class = "git_reflog")
}

##' @export
print.git_reflog <- function(x, ...) {
    lapply(x, print)
    invisible(x)
}

##' Print a reflog entry
##'
##' @param x The reflog entry
##' @param ... Unused
##' @return None (invisible 'NULL').
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user and commit a file
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## View repository HEAD reflog
##' reflog(repo)
##' }
print.git_reflog_entry <- function(x, ...) {
    cat(sprintf("[%s] %s@{%i}: %s\n",
                substring(x$sha, 1, 7),
                x$refname,
                x$index,
                x$message))
    invisible(x)
}
