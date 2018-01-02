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

##' Blobs in the object database
##'
##' List all blobs reachable from the commits in the object
##' database. For each commit, list blob's in the commit tree and
##' sub-trees.
##' @rdname odb_blobs-methods
##' @docType methods
##' @param repo The repository
##' @return A data.frame with the following columns:
##' \describe{
##'   \item{sha}{The sha of the blob}
##'   \item{path}{The path to the blob from the tree and sub-trees}
##'   \item{name}{The name of the blob from the tree that contains the blob}
##'   \item{len}{The length of the blob}
##'   \item{commit}{The sha of the commit}
##'   \item{author}{The author of the commit}
##'   \item{when}{The timestamp of the author signature in the commit}
##' }
##' @note A blob sha can have several entries
##' @keywords methods
##' @examples \dontrun{
##' ## Create a directory in tempdir
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##'
##' ## Initialize a repository
##' repo <- init(path)
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Create a file, add and commit
##' writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'            con = file.path(path, "test.txt"))
##' add(repo, "test.txt")
##' commit(repo, "Commit message 1")
##'
##' ## Change file and commit
##' writeLines(c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'              "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
##'              con = file.path(path, "test.txt"))
##' add(repo, "test.txt")
##' commit(repo, "Commit message 2")
##'
##' ## Commit same content under different name in a sub-directory
##' dir.create(file.path(path, "sub-directory"))
##' file.copy(file.path(path, "test.txt"), file.path(path, "sub-directory", "copy.txt"))
##' add(repo, "sub-directory/copy.txt")
##' commit(repo, "Commit message 3")
##'
##' ## List blobs
##' odb_blobs(repo)
##' }
setGeneric("odb_blobs",
           signature = "repo",
           function(repo)
           standardGeneric("odb_blobs"))

##' @rdname odb_blobs-methods
##' @export
setMethod("odb_blobs",
          signature(repo = "missing"),
          function()
          {
              callGeneric(repo = lookup_repository())
          }
)

##' @rdname odb_blobs-methods
##' @export
setMethod("odb_blobs",
          signature(repo = "git_repository"),
          function(repo)
          {
              blobs <- data.frame(.Call(git2r_odb_blobs, repo),
                                   stringsAsFactors = FALSE)
              blobs <- blobs[order(blobs$when),]
              index <- paste0(blobs$sha, ":", blobs$path, "/", blobs$name)
              blobs <- blobs[!duplicated(index),]
              rownames(blobs) <- NULL
              blobs$when <- as.POSIXct(blobs$when, origin="1970-01-01", tz="GMT")
              blobs
          }
)

##' List all objects available in the database
##'
##' @rdname odb_objects-methods
##' @docType methods
##' @param repo The repository
##' @return A data.frame with the following columns:
##' \describe{
##'   \item{sha}{The sha of the object}
##'   \item{type}{The type of the object}
##'   \item{len}{The length of the object}
##' }
##' @keywords methods
##' @examples \dontrun{
##' ## Create a directory in tempdir
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##'
##' ## Initialize a repository
##' repo <- init(path)
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Create a file, add and commit
##' writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'            con = file.path(path, "test.txt"))
##' add(repo, "test.txt")
##' commit(repo, "Commit message 1")
##'
##' ## Create tag
##' tag(repo, "Tagname", "Tag message")
##'
##' ## List objects in repository
##' odb_objects(repo)
##' }
setGeneric("odb_objects",
           signature = "repo",
           function(repo)
           standardGeneric("odb_objects"))

##' @rdname odb_objects-methods
##' @export
setMethod("odb_objects",
          signature(repo = "git_repository"),
          function(repo)
          {
              data.frame(.Call(git2r_odb_objects, repo),
                         stringsAsFactors = FALSE)
          }
)
