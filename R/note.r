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

##' Default notes reference
##'
##' Get the default notes reference for a repository
##' @rdname note_default_ref-methods
##' @docType methods
##' @param repo The repository
##' @return Character vector of length one with name of default notes
##' reference
##' @keywords methods
setGeneric("note_default_ref",
           signature = "repo",
           function(repo)
           standardGeneric("note_default_ref"))

##' @rdname note_default_ref-methods
##' @export
setMethod("note_default_ref",
          signature = "git_repository",
          function(repo)
          {
              .Call(git2r_note_default_ref, repo)
          }
)

##' Add note for a object
##'
##' @rdname note_create-methods
##' @docType methods
##' @param object The object to annotate
##' @param message Content of the note to add
##' @param ref Canonical name of the reference to use. Default is
##' \code{note_default_ref}.
##' @param author Signature of the notes note author
##' @param committer Signature of the notes note committer
##' @param force Overwrite existing note. Default is FALSE
##' @return S4 class git_note
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Create a directory in tempdir
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##'
##' ## Initialize a repository
##' repo <- init(path)
##'
##' ## Create a file, add and commit
##' writeLines("Hello world!", file.path(path, "test.txt"))
##' add(repo, "test.txt")
##' commit.1 <- commit(repo, "Commit message 1")
##'
##' ## Create another commit
##' writeLines(c("Hello world!",
##'              "HELLO WORLD!"),
##'            file.path(path, "test.txt"))
##' add(repo, "test.txt")
##' commit.2 <- commit(repo, "Commit message 2")
##'
##' ## Check that note_list is an empty list
##' stopifnot(identical(note_list(repo), list()))
##'
##' ## Create note in default namespace
##' note.1 <- note_create(commit.1, "Note-1")
##' note.2 <- note_create(commit.1, "Note-2", force = TRUE)
##'
##' ## Create note in named (review) namespace
##' note.3 <- note_create(commit.1, "Note-3", ref="refs/notes/review")
##' note.4 <- note_create(commit.2, "Note-4", ref="refs/notes/review")
##' note_remove(note.3)
##' note_remove(note.4)
##' note.5 <- note_create(commit.1, "Note-5", ref="review")
##' note.6 <- note_create(commit.2, "Note-6", ref="review")
##'
##' ## Create note on blob and tree
##' note.7 <- note_create(tree(commit.1), "Note-7")
##' note.8 <- note_create(tree(commit.1)["test.txt"], "Note-8")
##'
##' ## Cleanup
##' unlink(path, recursive=TRUE)
##' }
setGeneric("note_create",
           signature = "object",
           function(object,
                    message,
                    ref       = note_default_ref(object@repo),
                    author    = default_signature(object@repo),
                    committer = default_signature(object@repo),
                    force     = FALSE)
           standardGeneric("note_create"))

##' @rdname note_create-methods
##' @export
setMethod("note_create",
          signature = "git_blob",
          function(object,
                   message,
                   ref,
                   author,
                   committer,
                   force)
          {
              stopifnot(is.character(ref))
              stopifnot(identical(length(ref), 1L))
              if (!length(grep("^refs/notes/", ref)))
                  ref <- paste0("refs/notes/", ref)
              .Call(git2r_note_create,
                    object@repo,
                    object@sha,
                    message,
                    ref,
                    author,
                    committer,
                    force)
          }
)

##' @rdname note_create-methods
##' @export
setMethod("note_create",
          signature = "git_commit",
          function(object,
                   message,
                   ref,
                   author,
                   committer,
                   force)
          {
              stopifnot(is.character(ref))
              stopifnot(identical(length(ref), 1L))
              if (!length(grep("^refs/notes/", ref)))
                  ref <- paste0("refs/notes/", ref)
              .Call(git2r_note_create,
                    object@repo,
                    object@sha,
                    message,
                    ref,
                    author,
                    committer,
                    force)
          }
)

##' @rdname note_create-methods
##' @export
setMethod("note_create",
          signature = "git_tree",
          function(object,
                   message,
                   ref,
                   author,
                   committer,
                   force)
          {
              stopifnot(is.character(ref))
              stopifnot(identical(length(ref), 1L))
              if (!length(grep("^refs/notes/", ref)))
                  ref <- paste0("refs/notes/", ref)
              .Call(git2r_note_create,
                    object@repo,
                    object@sha,
                    message,
                    ref,
                    author,
                    committer,
                    force)
          }
)

##' List notes
##'
##' List all the notes within a specified namespace.
##' @rdname note_list-methods
##' @docType methods
##' @param repo The repository
##' @param ref Reference to read from. Default is
##' "refs/notes/commits".
##' @return list with S4 class git_note objects
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Create and initialize a repository in a temporary directory
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##' config(repo, user.name="User", user.email="user@@example.org")
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
##' note_list(repo)
##'
##' ## List notes in 'review' namespace
##' note_list(repo, "review")
##' }
setGeneric("note_list",
           signature = c("repo", "ref"),
           function(repo, ref)
           standardGeneric("note_list"))

##' @rdname note_list-methods
##' @export
setMethod("note_list",
          signature(repo = "git_repository",
                    ref  = "missing"),
          function(repo)
          {
              note_list(repo = repo, ref = note_default_ref(repo))
          }
)

##' @rdname note_list-methods
##' @export
setMethod("note_list",
          signature(repo = "git_repository",
                    ref  = "character"),
          function(repo, ref)
          {
              stopifnot(identical(length(ref), 1L))
              if (!length(grep("^refs/notes/", ref)))
                  ref <- paste0("refs/notes/", ref)
              .Call(git2r_note_list, repo, ref)
          }
)

##' Remove the note for an object
##'
##' @rdname note_remove-methods
##' @docType methods
##' @param note The note to remove
##' @param author Signature of the notes commit author.
##' @param committer Signature of the notes commit committer.
##' @return invisible NULL
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Create and initialize a repository in a temporary directory
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##' config(repo, user.name="User", user.email="user@@example.org")
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
##' note_list(repo)
##'
##' ## List notes in 'review' namespace
##' note_list(repo, "review")
##'
##' ## Remove notes
##' note_remove(note_1)
##' note_remove(note_2)
##'
##' ## List notes in default namespace
##' note_list(repo)
##'
##' ## List notes in 'review' namespace
##' note_list(repo, "review")
##' }
setGeneric("note_remove",
           signature = "note",
           function(note,
                    author    = default_signature(note@repo),
                    committer = default_signature(note@repo))
           standardGeneric("note_remove"))

##' @rdname note_remove-methods
##' @export
setMethod("note_remove",
          signature = "git_note",
          function(note,
                   author,
                   committer)
          {
              invisible(.Call(git2r_note_remove, note, author, committer))
          }
)
