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

##' Class \code{"git_tree"}
##'
##' @title S4 class to handle a git tree
##' @section Slots:
##' \describe{
##'   \item{hex}{
##'     40 char hexadecimal string
##'   }
##'   \item{filemode}{
##'     The UNIX file attributes of a tree entry
##'   }
##'   \item{type}{
##'     String representation of the tree entry type
##'   }
##'   \item{id}{
##'     The hex id of a tree entry
##'   }
##'   \item{name}{
##'     The filename of a tree entry
##'   }
##'   \item{repo}{
##'     The S4 class git_repository that contains the commit
##'   }
##' }
##' @name git_tree-class
##' @docType class
##' @keywords classes
##' @keywords methods
##' @include repository.r
##' @export
setClass("git_tree",
         slots=c(hex      = "character",
                 filemode = "integer",
                 type     = "character",
                 id       = "character",
                 name     = "character",
                 repo     = "git_repository"),
         validity=function(object)
         {
             errors <- character(0)

             if(!identical(length(object@hex), 1L))
                 errors <- c(errors, "hex must have length equal to one")

             if(length(errors) == 0) TRUE else errors
         }
)

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
##'   \item{id}{
##'     The hex id of a tree entry
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
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Coerce tree to a data.frame
##' df <- as(tree(commits(repo)[[1]]), "data.frame")
##'
##' str(df)
##' }
setAs(from = "git_tree",
      to = "data.frame",
      def = function(from)
      {
          data.frame(mode = sprintf("%06o", from@filemode),
                     type = from@type,
                     id   = from@id,
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
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Inspect size of each blob in tree
##' invisible(lapply(as(tree(commits(repo)[[1]]), "list"),
##'   function(obj) {
##'     if(is_blob(obj))
##'       summary(obj)
##'     NULL
##'   }))
##' }
setAs(from = "git_tree",
      to = "list",
      def = function(from)
      {
          lapply(from@id, function(hex) lookup(from@repo, hex))
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
##' @include commit.r
setGeneric("tree",
           signature = "object",
           function(object) standardGeneric("tree"))

##' @rdname tree-methods
##' @export
setMethod("tree",
          signature(object = "git_commit"),
          function (object)
          {
              .Call("git2r_commit_tree", object)
          }
)

##' @rdname tree-methods
##' @include stash.r
##' @export
setMethod("tree",
          signature(object = "git_stash"),
          function (object)
          {
              .Call("git2r_commit_tree", object)
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
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Brief summary of one tree in repository
##' tree(commits(repo)[[1]])
##' }
##'
setMethod("show",
          signature(object = "git_tree"),
          function (object)
          {
              cat(sprintf("tree:  %s\n", object@hex))
          }
)

##' Summary of tree
##'
##' @aliases summary,git_tree-methods
##' @docType methods
##' @param object The tree \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
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
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Pick a tree in the repository
##' tree_object <- tree(commits(repo)[[1]])
##'
##' ## Summarize tree
##' summary(tree_object)
##'
##' ## Select item by name
##' tree_object[".Rbuildignore"]
##'
##' ## Select first item in tree
##' tree_object[1]
##'
##' ## Select first three items in tree
##' tree_object[1:3]
##'
##' ## Select all blobs in tree
##' tree_object[sapply(as(tree_object, 'list'), is_blob)]
##' }
setMethod("[",
          signature(x = "git_tree", i = "integer", j = "missing"),
          function(x, i)
          {
              i <- seq_len(length(x))[i]
              ret <- lapply(i, function(j) lookup(x@repo, x@id[j]))
              if(identical(length(ret), 1L))
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
              x[seq_len(length(x))[i]]
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
