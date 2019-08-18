## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2018 The git2r contributors
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

##' Get the SHA-1 of a git object
##'
##' Get the 40 character hexadecimal string of the SHA-1.
##' @param object a git object to get the SHA-1 from.
##' @return The 40 character hexadecimal string of the SHA-1.
##' @export
##' @examples \dontrun{
##' ## Create a directory in tempdir
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##'
##' ## Initialize a repository
##' repo <- init(path)
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Create a file, add and commit
##' lines <- "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do"
##' writeLines(lines, file.path(path, "test.txt"))
##' add(repo, "test.txt")
##' commit(repo, "Commit message 1")
##'
##' ## Get the SHA-1 of the last commit
##' sha(last_commit(repo))
##' }
sha <- function(object) {
    UseMethod("sha", object)
}

##' @rdname sha
##' @export
sha.git_blob <- function(object) {
    object$sha
}

##' @rdname sha
##' @export
sha.git_branch <- function(object) {
    branch_target(object)
}

##' @rdname sha
##' @export
sha.git_commit <- function(object) {
    object$sha
}

##' @rdname sha
##' @export
sha.git_note <- function(object) {
    object$sha
}

##' @rdname sha
##' @export
sha.git_reference <- function(object) {
    object$sha
}

##' @rdname sha
##' @export
sha.git_reflog_entry <- function(object) {
    object$sha
}

##' @rdname sha
##' @export
sha.git_tag <- function(object) {
    object$sha
}

##' @rdname sha
##' @export
sha.git_tree <- function(object) {
    object$sha
}

##' @rdname sha
##' @export
sha.git_fetch_head <- function(object) {
    object$sha
}

##' @rdname sha
##' @export
sha.git_merge_result <- function(object) {
    object$sha
}
