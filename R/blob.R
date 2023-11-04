## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2023 The git2r contributors
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
##' @param repo The repository where the blob(s) will be written. Can
##'     be a bare repository. A \code{git_repository} object, or a
##'     path to a repository, or \code{NULL}.  If the \code{repo}
##'     argument is \code{NULL}, the repository is searched for with
##'     \code{\link{discover_repository}} in the current working
##'     directory.
##' @param path The file(s) from which the blob will be created.
##' @param relative TRUE if the file(s) from which the blob will be
##'     created is relative to the repository's working dir. Default
##'     is TRUE.
##' @return list of S3 class git_blob \code{objects}
##' @export
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
##' blob_list_2 <- blob_create(repo, c(temp_file_1, temp_file_2),
##'                            relative = FALSE)
##' }
blob_create <- function(repo = ".", path = NULL, relative = TRUE) {
    repo <- lookup_repository(repo)
    if (isTRUE(relative))
        return(.Call(git2r_blob_create_fromworkdir, repo, path))
    path <- normalizePath(path, mustWork = TRUE)
    .Call(git2r_blob_create_fromdisk, repo, path)
}

##' Content of blob
##'
##' @param blob The blob object.
##' @param split Split blob content to text lines. Default TRUE.
##' @param raw When \code{TRUE}, get the content of the blob as a raw
##'     vector, else as a character vector. Default is \code{FALSE}.
##' @return The content of the blob. NA_character_ if the blob is
##'     binary and \code{raw} is \code{FALSE}.
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
##' ## Display content of blob.
##' content(tree(commits(repo)[[1]])["example.txt"])
##' }
content <- function(blob = NULL, split = TRUE, raw = FALSE) {
    result <- .Call(git2r_blob_content, blob, raw)
    if (isTRUE(raw))
        return(result)
    if (isTRUE(split))
        result <- strsplit(result, "\n")[[1]]
    result
}

##' Determine the sha from a blob string
##'
##' The blob is not written to the object database.
##' @param data The string vector to hash.
##' @return A string vector with the sha for each string in data.
##' @export
##' @examples
##' \dontrun{
##' identical(hash(c("Hello, world!\n",
##'                  "test content\n")),
##'                c("af5626b4a114abcb82d63db7c8082c3c4756e51b",
##'                  "d670460b4b4aece5915caf5c68d12f560a9fe3e4"))
##' }
hash <- function(data = NULL) {
    .Call(git2r_odb_hash, data)
}

##' Determine the sha from a blob in a file
##'
##' The blob is not written to the object database.
##' @param path The path vector with files to hash.
##' @return A vector with the sha for each file in path.
##' @export
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
hashfile <- function(path = NULL) {
    path <- normalizePath(path, mustWork = TRUE)
    if (any(is.na(path)))
        stop("Invalid 'path' argument")
    .Call(git2r_odb_hashfile, path)
}

##' Is blob binary
##'
##' @param blob The blob \code{object}.
##' @return TRUE if binary data, FALSE if not.
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
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
is_binary <- function(blob = NULL) {
    .Call(git2r_blob_is_binary, blob)
}

##' Check if object is S3 class git_blob
##'
##' @param object Check if object is S3 class git_blob
##' @return TRUE if object is S3 class git_blob, else FALSE
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
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
    inherits(object, "git_blob")
}

##' Size in bytes of the contents of a blob
##'
##' @param x The blob \code{object}
##' @return a non-negative integer
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
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
length.git_blob <- function(x) {
    .Call(git2r_blob_rawsize, x)
}

##' @export
format.git_blob <- function(x, ...) {
    sprintf("blob:  %s\nsize:  %i bytes", x$sha, length(x))
}

##' @export
print.git_blob <- function(x, ...) {
    cat(format(x, ...), "\n", sep = "")
    invisible(x)
}
