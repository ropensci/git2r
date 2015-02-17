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

##' Add file(s) to index
##'
##' @rdname add-methods
##' @docType methods
##' @param repo The repository \code{object}.
##' @param path character vector with filenames to add. The path must
##' be relative to the repository's working folder. Only non-ignored
##' files are added. If path is a directory, files in sub-folders are
##' added (if non-ignored)
##' @return invisible(NULL)
##' @keywords methods
##' @include S4_classes.r
##' @examples
##' \dontrun{
##' ## Initialize a repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Create a file
##' writeLines("Hello world!", file.path(path, "file-to-add.txt"))
##'
##' ## Add file to repository
##' add(repo, "file-to-add.txt")
##'
##' ## View status of repository
##' status(repo)
##' }
##'
setGeneric("add",
           signature = c("repo", "path"),
           function(repo, path)
           standardGeneric("add"))

##' @rdname add-methods
##' @export
setMethod("add",
          signature(repo = "git_repository",
                    path = "character"),
          function (repo, path)
          {
              .Call(git2r_index_add_all, repo, path)
              invisible(NULL)
          }
)
