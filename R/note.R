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

##' Default notes reference
##'
##' Get the default notes reference for a repository
##' @template repo-param
##' @return Character vector of length one with name of default notes
##'     reference
##' @export
##' @examples
##' \dontrun{
##' ## Create and initialize a repository in a temporary directory
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## View default notes reference
##' note_default_ref(repo)
##' }
note_default_ref <- function(repo = ".") {
    .Call(git2r_note_default_ref, lookup_repository(repo))
}

##' Add note for a object
##'
##' @param object The object to annotate (git_blob, git_commit or
##'     git_tree).
##' @param message Content of the note to add
##' @param ref Canonical name of the reference to use. Default is
##'     \code{note_default_ref}.
##' @param author Signature of the notes note author
##' @param committer Signature of the notes note committer
##' @param force Overwrite existing note. Default is FALSE
##' @return git_note
##' @export
##' @examples
##' \dontrun{
##' ## Create and initialize a repository in a temporary directory
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Create a file, add and commit
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit_1 <- commit(repo, "Commit message 1")
##'
##' ## Create another commit
##' writeLines(c("Hello world!",
##'              "HELLO WORLD!"),
##'            file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit_2 <- commit(repo, "Commit message 2")
##'
##' ## Check that notes is an empty list
##' notes(repo)
##'
##' ## Create note in default namespace
##' note_create(commit_1, "Note-1")
##'
##' ## Create note in named (review) namespace
##' note_create(commit_1, "Note-2", ref="refs/notes/review")
##' note_create(commit_2, "Note-3", ref="review")
##'
##' ## Create note on blob and tree
##' note_create(tree(commit_1), "Note-4")
##' note_create(tree(commit_1)["example.txt"], "Note-5")
##' }
note_create <- function(object    = NULL,
                        message   = NULL,
                        ref       = NULL,
                        author    = NULL,
                        committer = NULL,
                        force     = FALSE) {
    if (is.null(object))
        stop("'object' is missing")
    if (!any(is_blob(object), is_commit(object), is_tree(object)))
        stop("'object' must be a 'git_blob', 'git_commit' or 'git_tree' object")

    repo <- object$repo
    sha <- object$sha
    if (is.null(ref))
        ref <- note_default_ref(repo)
    stopifnot(is.character(ref), identical(length(ref), 1L))
    if (!length(grep("^refs/notes/", ref)))
        ref <- paste0("refs/notes/", ref)
    if (is.null(author))
        author <- default_signature(repo)
    if (is.null(committer))
        committer <- default_signature(repo)
    .Call(git2r_note_create, repo, sha, message, ref, author, committer, force)
}

##' List notes
##'
##' List all the notes within a specified namespace.
##' @template repo-param
##' @param ref Reference to read from. Default (ref = NULL) is to call
##'     \code{note_default_ref}.
##' @return list with git_note objects
##' @export
##' @examples
##' \dontrun{
##' ## Create and initialize a repository in a temporary directory
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Create a file, add and commit
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit_1 <- commit(repo, "Commit message 1")
##'
##' ## Create another commit
##' writeLines(c("Hello world!",
##'              "HELLO WORLD!"),
##'            file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit_2 <- commit(repo, "Commit message 2")
##'
##' ## Create note in default namespace
##' note_create(commit_1, "Note-1")
##' note_create(commit_1, "Note-2", force = TRUE)
##'
##' ## Create note in named (review) namespace
##' note_create(commit_1, "Note-3", ref="refs/notes/review")
##' note_create(commit_2, "Note-4", ref="review")
##'
##' ## Create note on blob and tree
##' note_create(tree(commit_1), "Note-5")
##' note_create(tree(commit_1)["example.txt"], "Note-6")
##'
##' ## List notes in default namespace
##' notes(repo)
##'
##' ## List notes in 'review' namespace
##' notes(repo, "review")
##' }
notes <- function(repo = ".", ref = NULL) {
    repo <- lookup_repository(repo)
    if (is.null(ref))
        ref <- note_default_ref(repo)
    stopifnot(is.character(ref), identical(length(ref), 1L))
    if (!length(grep("^refs/notes/", ref)))
        ref <- paste0("refs/notes/", ref)
    .Call(git2r_notes, repo, ref)
}

##' Remove the note for an object
##'
##' @param note The note to remove
##' @param author Signature of the notes commit author.
##' @param committer Signature of the notes commit committer.
##' @return invisible NULL
##' @export
##' @examples
##' \dontrun{
##' ## Create and initialize a repository in a temporary directory
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Create a file, add and commit
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit_1 <- commit(repo, "Commit message 1")
##'
##'
##' ## Create note in default namespace
##' note_1 <- note_create(commit_1, "Note-1")
##'
##' ## Create note in named (review) namespace
##' note_2 <- note_create(commit_1, "Note-2", ref="refs/notes/review")
##'
##' ## List notes in default namespace
##' notes(repo)
##'
##' ## List notes in 'review' namespace
##' notes(repo, "review")
##'
##' ## Remove notes
##' note_remove(note_1)
##' note_remove(note_2)
##'
##' ## List notes in default namespace
##' notes(repo)
##'
##' ## List notes in 'review' namespace
##' notes(repo, "review")
##' }
note_remove <- function(note      = NULL,
                        author    = NULL,
                        committer = NULL) {
    if (!inherits(note, "git_note"))
        stop("'note' is not a git_note")
    if (is.null(author))
        author <- default_signature(note$repo)
    if (is.null(committer))
        committer <- default_signature(note$repo)
    .Call(git2r_note_remove, note, author, committer)
    invisible(NULL)
}

##' @export
format.git_note <- function(x, ...) {
    sprintf("note:  %s", x$sha)
}

##' @export
print.git_note <- function(x, ...) {
    cat(format(x, ...), "\n", sep = "")
    invisible(x)
}
