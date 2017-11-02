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

##' Create blob from file on disk
##'
##' Read a file from the filesystem and write its content to the
##' Object Database as a loose blob. The method is vectorized and
##' accepts a vector of files to create blobs from.
##' @rdname blob_create-methods
##' @docType methods
##' @param repo The repository where the blob(s) will be written. Can
##' be a bare repository.
##' @param path The file(s) from which the blob will be created.
##' @param relative TRUE if the file(s) from which the blob will be
##' created is relative to the repository's working dir. Default is
##' TRUE.
##' @return list of S4 class git_blob \code{objects}
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create blobs from files relative to workdir
##' writeLines("Hello, world!", file.path(path, "example-1.txt"))
##' writeLines("test content", file.path(path, "example-2.txt"))
##' blob_list_1 <- blob_create(repo, c("example-1.txt",
##'                                    "example-2.txt"))
##'
##' ## Create blobs from files not relative to workdir
##' temp_file_1 <- tempfile()
##' temp_file_2 <- tempfile()
##' writeLines("Hello, world!", temp_file_1)
##' writeLines("test content", temp_file_2)
##' blob_list_2 <- blob_create(repo, c(temp_file_1, temp_file_2), relative = FALSE)
##' }
setGeneric("blob_create",
           signature = c("repo", "path"),
           function(repo, path, relative = TRUE)
           standardGeneric("blob_create"))

##' @rdname blob_create-methods
##' @export
setMethod("blob_create",
          signature(repo = "git_repository",
                    path = "character"),
          function(repo, path, relative)
          {
              ## Argument checking
              stopifnot(is.logical(relative),
                        identical(length(relative), 1L))

              if (relative) {
                  result <- .Call(git2r_blob_create_fromworkdir,
                                  repo,
                                  path)
              } else {
                  path <- normalizePath(path, mustWork = TRUE)
                  result <- .Call(git2r_blob_create_fromdisk,
                                  repo,
                                  path)
              }

              result
          }
)

##' Content of blob
##'
##' @rdname content-methods
##' @docType methods
##' @param blob The blob \code{object}.
##' @param split Split blob content to text lines. Default TRUE.
##' @return The content of the blob. NA_character_ if the blob is binary.
##' @keywords methods
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
##' ## Display content of blob.
##' content(tree(commits(repo)[[1]])["example.txt"])
##' }
setGeneric("content",
           signature = "blob",
           function(blob,
                    split = TRUE)
           standardGeneric("content"))

##' @rdname content-methods
##' @export
setMethod("content",
          signature(blob = "git_blob"),
          function(blob, split)
          {
              if (is_binary(blob))
                  return(NA_character_)

              ret <- .Call(git2r_blob_content, blob)
              if (isTRUE(split))
                  ret <- strsplit(ret, "\n")[[1]]
              ret
          }
)

##' Determine the sha from a blob string
##'
##' The blob is not written to the object database.
##' @rdname hash-methods
##' @docType methods
##' @param data The string vector to hash.
##' @return A string vector with the sha for each string in data.
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
              .Call(git2r_odb_hash, data)
          }
)

##' Determine the sha from a blob in a file
##'
##' The blob is not written to the object database.
##' @rdname hashfile-methods
##' @docType methods
##' @param path The path vector with files to hash.
##' @return A vector with the sha for each file in path.
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Create a file. NOTE: The line endings from writeLines gives
##' ## LF (line feed) on Unix/Linux and CRLF (carriage return, line feed)
##' ## on Windows. The example use writeChar to have more control.
##' path <- tempfile()
##' f <- file(path, "wb")
##' writeChar("Hello, world!\n", f, eos = NULL)
##' close(f)
##'
##' ## Generate hash
##' hashfile(path)
##' identical(hashfile(path), hash("Hello, world!\n"))
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
              .Call(git2r_odb_hashfile, path)
          }
)

##' Is blob binary
##'
##' @rdname is_binary-methods
##' @docType methods
##' @param blob The blob \code{object}.
##' @return TRUE if binary data, FALSE if not.
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Commit a text file
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit_1 <- commit(repo, "First commit message")
##'
##' ## Check if binary
##' b_text <- tree(commit_1)["example.txt"]
##' is_binary(b_text)
##'
##' ## Commit plot file (binary)
##' x <- 1:100
##' y <- x^2
##' png(file.path(path, "plot.png"))
##' plot(y ~ x, type = "l")
##' dev.off()
##' add(repo, "plot.png")
##' commit_2 <- commit(repo, "Second commit message")
##'
##' ## Check if binary
##' b_png <- tree(commit_2)["plot.png"]
##' is_binary(b_png)
##' }
setGeneric("is_binary",
           signature = "blob",
           function(blob)
           standardGeneric("is_binary"))

##' @rdname is_binary-methods
##' @export
setMethod("is_binary",
          signature(blob = "git_blob"),
          function(blob)
          {
              .Call(git2r_blob_is_binary, blob)
          }
)

##' Check if object is S4 class git_blob
##'
##' @param object Check if object is S4 class git_blob
##' @return TRUE if object is S4 class git_blob, else FALSE
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Commit a text file
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit_1 <- commit(repo, "First commit message")
##' blob_1 <- tree(commit_1)["example.txt"]
##'
##' ## Check if blob
##' is_blob(commit_1)
##' is_blob(blob_1)
##' }
is_blob <- function(object) {
    is(object = object, class2 = "git_blob")
}

##' Size in bytes of the contents of a blob
##'
##' @docType methods
##' @param x The blob \code{object}
##' @return a non-negative integer
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Commit a text file
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit_1 <- commit(repo, "First commit message")
##' blob_1 <- tree(commit_1)["example.txt"]
##'
##' ## Get length in size of bytes of the content of the blob
##' length(blob_1)
##' }
setMethod("length",
          signature(x = "git_blob"),
          function(x)
          {
              .Call(git2r_blob_rawsize, x)
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
##' ## Brief summary of the blob in the repository
##' tree(commits(repo)[[1]])["example.txt"]
##' }
setMethod("show",
          signature(object = "git_blob"),
          function(object)
          {
              cat(sprintf("blob:  %s\n", object@sha))
          }
)

##' Summary of blob
##'
##' @docType methods
##' @param object The blob \code{object}
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
##' ## Create a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Commit a text file
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit_1 <- commit(repo, "First commit message")
##' blob_1 <- tree(commit_1)["example.txt"]
##'
##' ## Get summary of the blob
##' summary(blob_1)
##' }
setMethod("summary",
          signature(object = "git_blob"),
          function(object, ...)
          {
              cat(sprintf("blob:  %s\nsize:  %i bytes\n",
                          object@sha,
                          length(object)))
          }
)
