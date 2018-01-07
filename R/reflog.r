## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2016 The git2r contributors
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
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Write to a file and commit
##' writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'            file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Change file and commit
##' writeLines(c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'              "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
##'            file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "Second commit message")
##'
##' ## Change file again and commit
##' writeLines(c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'              "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad",
##'              "minim veniam, quis nostrud exercitation ullamco laboris nisi ut"),
##'            file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "Third commit message")
##'
##' ## View reflog
##' reflog(repo)
##' }
reflog <- function(repo = NULL, refname = "HEAD") {
    structure(.Call(git2r_reflog_list, repo, refname), class = "git_reflog")
}

##' @export
print.git_reflog <- function(x, ...)
{
    lapply(x, show)
    invisible(NULL)
}

##' Brief summary of a reflog entry
##'
##' @aliases show,git_reflog_entry-methods
##' @docType methods
##' @param object The reflog entry \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
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
##' ## View repository HEAD reflog
##' reflog(repo)
##' }
setMethod("show",
          signature(object = "git_reflog_entry"),
          function(object)
          {
              cat(sprintf("[%s] %s@{%i}: %s\n",
                          substring(object@sha, 1, 7),
                          object@refname,
                          object@index,
                          object@message))
          }
)
