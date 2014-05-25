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

##' Class \code{"git_blob"}
##'
##' @title  S4 class to handle a git blob
##' @section Slots:
##' \describe{
##'   \item{hex}{
##'     40 char hexadecimal string
##'   }
##'   \item{repo}{
##'     The S4 class git_repository that contains the blob
##'   }
##' }
##' @rdname git_blob-class
##' @docType class
##' @keywords classes
##' @keywords methods
##' @include repository.r
##' @export
setClass("git_blob",
         slots=c(hex  = "character",
                 repo = "git_repository"),
         validity=function(object) {
             errors <- character()

             if (length(errors) == 0) TRUE else errors
         }
)

##' Content of blob
##'
##' @rdname content-methods
##' @docType methods
##' @param object The blob \code{object}.
##' @param split Split blob content to text lines. Default TRUE.
##' @return The content of the blob
##' @keywords methods
setGeneric("content",
           signature = "blob",
           function(blob,
                    split = TRUE)
           standardGeneric("content"))

##' @rdname content-methods
##' @export
setMethod("content",
          signature(blob = "git_blob"),
          function (blob, split)
          {
              if(is.binary(blob))
                  stop("Content of binary blob is not supported (yet)")
              
              ret <- .Call("git2r_blob_content", blob)
              if(identical(split, TRUE))
                  ret <- strsplit(ret, "\n")[[1]]
              ret
          }
)

##' Determine the sha1 hex of a blob from a string
##'
##' The blob is not written to the object database.
##' @rdname hash-methods
##' @docType methods
##' @param data The string vector to hash.
##' @return A string vector with the sha1 hex for each string in data.
##' @keywords methods
##' @examples
##' \dontrun{
##' identical(hash(c("Hello, world!\n",
##'                  "test content\n")),
##'                c("af5626b4a114abcb82d63db7c8082c3c4756e51b",
##'                  "d670460b4b4aece5915caf5c68d12f560a9fe3e4"))
##' }
setGeneric("hash",
           signature = "data",
           function(data)
           standardGeneric("hash"))

##' @rdname hash-methods
##' @export
setMethod("hash",
          signature(data = "character"),
          function(data)
          {
              .Call("git2r_odb_hash", data)
          }
)

##' Determine the sha1 hex of a blob from a file
##'
##' The blob is not written to the object database.
##' @rdname hashfile-methods
##' @docType methods
##' @param path The path vector with files to hash.
##' @return A path vector with the sha1 hex for each file in path.
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Create a file
##' path <- tempfile()
##' writeLines("Hello, world!", path)
##'
##' hashfile(path)
##' identical(hashfile(path), hash("Hello, world!\n"))
##'
##' ## Delete file
##' unlink(path)
##' }
setGeneric("hashfile",
           signature = "path",
           function(path)
           standardGeneric("hashfile"))

##' @rdname hashfile-methods
##' @export
setMethod("hashfile",
          signature(path = "character"),
          function(path)
          {
              path <- normalizePath(path, mustWork = TRUE)
              .Call("git2r_odb_hashfile", path)
          }
)

##' Is blob binary
##'
##' @rdname is.binary-methods
##' @docType methods
##' @param object The blob \code{object}.
##' @return TRUE if binary data, FALSE if not.
##' @keywords methods
setGeneric("is.binary",
           signature = "object",
           function(object)
           standardGeneric("is.binary"))

##' @rdname is.binary-methods
##' @export
setMethod("is.binary",
          signature(object = "git_blob"),
          function (object)
          {
              .Call("git2r_blob_is_binary", object)
          }
)

##' Check if object is S4 class git_blob
##'
##' @param object Check if object is S4 class git_blob
##' @return TRUE if object is S4 class git_blob, else FALSE
##' @keywords methods
##' @export
is.blob <- function(object) {
    is(object = object, class2 = "git_blob")
}

##' Size in bytes of the contents of a blob
##'
##' @docType methods
##' @param x The blob \code{object}
##' @return a non-negative integer
##' @keywords methods
##' @export
setMethod("length",
          signature(x = "git_blob"),
          function(x)
          {
              .Call("git2r_blob_rawsize", x)
          }
)

##' Brief summary of blob
##'
##' @aliases show,git_blob-methods
##' @docType methods
##' @param object The blob \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
setMethod("show",
          signature(object = "git_blob"),
          function (object)
          {
              cat(sprintf("blob:  %s\n", object@hex))
          }
)

##' Summary of blob
##'
##' @docType methods
##' @param object The blob \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
setMethod("summary",
          signature(object = "git_blob"),
          function(object, ...)
          {
              cat(sprintf("blob:  %s\nsize:  %i bytes\n",
                          object@hex,
                          length(object)))
          }
)
