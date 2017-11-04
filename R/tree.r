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

##' Coerce entries in a git_tree to a \code{data.frame}
##'
##' The entries in a tree are coerced to a \code{data.frame}
##'
##'
##' The \code{data.frame} have the following columns:
##' \describe{
##'
##'   \item{filemode}{
##'     The UNIX file attributes of a tree entry
##'   }
##'
##'   \item{type}{
##'     String representation of the tree entry type
##'   }
##'
##'   \item{sha}{
##'     The sha of a tree entry
##'   }
##'
##'   \item{name}{
##'     The filename of a tree entry
##'   }
##'
##' }
##' @name coerce-git_tree-data.frame-method
##' @aliases coerce,git_tree,data.frame-method
##' @docType methods
##' @param from The tree \code{object}
##' @return \code{data.frame}
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' dir.create(file.path(path, "subfolder"))
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Create three files and commit
##' writeLines("First file",  file.path(path, "example-1.txt"))
##' writeLines("Second file", file.path(path, "subfolder/example-2.txt"))
##' writeLines("Third file",  file.path(path, "example-3.txt"))
##' add(repo, c("example-1.txt", "subfolder/example-2.txt", "example-3.txt"))
##' new_commit <- commit(repo, "Commit message")
##'
##' ## Coerce tree to a data.frame
##' df <- as(tree(new_commit), "data.frame")
##' df
##' }
setAs(from = "git_tree",
      to = "data.frame",
      def = function(from)
      {
          data.frame(mode = sprintf("%06o", from@filemode),
                     type = from@type,
                     sha  = from@id,
                     name = from@name,
                     stringsAsFactors = FALSE)
      }
)

##' Coerce entries in a git_tree to a list of entry objects
##'
##' @name coerce-git_tree-list-method
##' @docType methods
##' @param from The tree \code{object}
##' @return list of entry objects
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' dir.create(file.path(path, "subfolder"))
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Create three files and commit
##' writeLines("First file",  file.path(path, "example-1.txt"))
##' writeLines("Second file", file.path(path, "subfolder/example-2.txt"))
##' writeLines("Third file",  file.path(path, "example-3.txt"))
##' add(repo, c("example-1.txt", "subfolder/example-2.txt", "example-3.txt"))
##' new_commit <- commit(repo, "Commit message")
##'
##' ## Inspect size of each blob in tree
##' invisible(lapply(as(tree(new_commit), "list"),
##'   function(obj) {
##'     if (is_blob(obj))
##'       summary(obj)
##'     NULL
##'   }))
##' }
setAs(from = "git_tree",
      to = "list",
      def = function(from)
      {
          lapply(from@id, function(sha) lookup(from@repo, sha))
      }
)

##' Tree
##'
##' Get the tree pointed to by a commit or stash.
##' @rdname tree-methods
##' @docType methods
##' @param object the \code{commit} or \code{stash} object
##' @return A S4 class git_tree object
##' @keywords methods
##' @include S4_classes.r
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a first user and commit a file
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' tree(commits(repo)[[1]])
##' summary(tree(commits(repo)[[1]]))
##' }
setGeneric("tree",
           signature = "object",
           function(object) standardGeneric("tree"))

##' @rdname tree-methods
##' @export
setMethod("tree",
          signature(object = "git_commit"),
          function(object)
          {
              .Call(git2r_commit_tree, object)
          }
)

##' @rdname tree-methods
##' @export
setMethod("tree",
          signature(object = "git_stash"),
          function(object)
          {
              .Call(git2r_commit_tree, object)
          }
)

##' Brief summary of tree
##'
##' @aliases show,git_tree-methods
##' @docType methods
##' @param object The tree \code{object}
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
##' ## Brief summary of the tree in the repository
##' tree(commits(repo)[[1]])
##' }
setMethod("show",
          signature(object = "git_tree"),
          function(object)
          {
              cat(sprintf("tree:  %s\n", object@sha))
          }
)

##' Summary of tree
##'
##' @aliases summary,git_tree-methods
##' @docType methods
##' @param object The tree \code{object}
##' @param ... Additional arguments affecting the summary produced.
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
##' summary(tree(commits(repo)[[1]]))
##' }
setMethod("summary",
          signature(object = "git_tree"),
          function(object, ...)
          {
              show(as(object, "data.frame"))
          }
)

##' Extract object from tree
##'
##' Lookup a tree entry by its position in the tree
##' @rdname tree-index-methods
##' @docType methods
##' @param x The tree \code{object}
##' @param i The index (integer or logical) of the tree object to
##' extract. If negative values, all elements except those indicated
##' are selected. A character vector to match against the names of
##' objects to extract.
##' @return Git object
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' dir.create(file.path(path, "subfolder"))
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Create three files and commit
##' writeLines("First file",  file.path(path, "example-1.txt"))
##' writeLines("Second file", file.path(path, "subfolder/example-2.txt"))
##' writeLines("Third file",  file.path(path, "example-3.txt"))
##' add(repo, c("example-1.txt", "subfolder/example-2.txt", "example-3.txt"))
##' new_commit <- commit(repo, "Commit message")
##'
##' ## Pick a tree in the repository
##' tree_object <- tree(new_commit)
##'
##' ## Summarize tree
##' summary(tree_object)
##'
##' ## Select item by name
##' tree_object["example-1.txt"]
##'
##' ## Select first item in tree
##' tree_object[1]
##'
##' ## Select first three items in tree
##' tree_object[1:3]
##'
##' ## Select all blobs in tree
##' tree_object[vapply(as(tree_object, 'list'), is_blob, logical(1))]
##' }
setMethod("[",
          signature(x = "git_tree", i = "integer", j = "missing"),
          function(x, i)
          {
              i <- seq_len(length(x))[i]
              ret <- lapply(i, function(j) lookup(x@repo, x@id[j]))
              if (identical(length(ret), 1L))
                  ret <- ret[[1]]
              ret
          }
)

##' @rdname tree-index-methods
##' @export
setMethod("[",
          signature(x = "git_tree", i = "numeric", j = "missing"),
          function(x, i)
          {
              x[as.integer(i)]
          }
)

##' @rdname tree-index-methods
##' @export
setMethod("[",
          signature(x = "git_tree", i = "logical", j = "missing"),
          function(x, i)
          {
              x[seq_along(x)[i]]
          }
)

##' @rdname tree-index-methods
##' @export
setMethod("[",
          signature(x = "git_tree", i = "character", j = "missing"),
          function(x, i)
          {
              x[which(x@name %in% i)]
          }
)

##' Number of entries in tree
##'
##' @docType methods
##' @param x The tree \code{object}
##' @return a non-negative integer or double (which will be rounded
##' down)
##' @keywords methods
##' @export
setMethod("length",
          signature(x = "git_tree"),
          function(x)
          {
              length(x@id)
          }
)
