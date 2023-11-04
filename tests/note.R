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

library("git2r")

## For debugging
sessionInfo()
libgit2_version()
libgit2_features()


## Create a directory in tempdir
path <- tempfile(pattern = "git2r-")
dir.create(path)

## Initialize a repository
repo <- init(path)
config(repo, user.name = "Alice", user.email = "alice@example.org")

## Create a file, add and commit
writeLines("Hello world!", file.path(path, "test.txt"))
add(repo, "test.txt")
commit_1 <- commit(repo, "Commit message 1")

## Create another commit
writeLines(c("Hello world!",
             "HELLO WORLD!"),
           file.path(path, "test.txt"))
add(repo, "test.txt")
commit_2 <- commit(repo, "Commit message 2")

## Check default ref
stopifnot(identical(note_default_ref(repo),
                    "refs/notes/commits"))

## Check that an invalid object argument in note_create produce an
## error.
tools::assertError(note_create(object = NULL, message = "test"))
tools::assertError(note_create(object = 1, message = "test"))

## Check that notes is an empty list
stopifnot(identical(notes(repo), list()))

## Create note in default namespace
note_1 <- note_create(commit_1, "Note-1")
stopifnot(identical(print(note_1), note_1))
stopifnot(identical(length(notes(repo)), 1L))
stopifnot(identical(sha(note_1), note_1$sha))
tools::assertError(note_create(commit_1, "Note-2"))
note_2 <- note_create(commit_1, "Note-2", force = TRUE)
stopifnot(identical(length(notes(repo)), 1L))

## Check that an invalid note argument in note_remove produce an
## error.
tools::assertError(note_remove(note = 1))

## Create note in named (review) namespace
note_3 <- note_create(commit_1, "Note-3", ref = "refs/notes/review")
note_4 <- note_create(commit_2, "Note-4", ref = "refs/notes/review")
stopifnot(identical(length(notes(repo, ref = "refs/notes/review")), 2L))
note_remove(note_3)
note_remove(note_4)
stopifnot(identical(notes(repo, ref = "refs/notes/review"), list()))
note_5 <- note_create(commit_1, "Note-5", ref = "review")
note_6 <- note_create(commit_2, "Note-6", ref = "review")
stopifnot(identical(length(notes(repo, ref = "review")), 2L))
note_remove(note_5)
note_remove(note_6)
stopifnot(identical(length(notes(repo, ref = "review")), 0L))

## Create note on blob and tree
tree_1 <- tree(commit_1)
note_7 <- note_create(tree_1, "Note-7")
stopifnot(is(object = lookup(repo, note_7$annotated), class2 = "git_tree"))
stopifnot(identical(length(notes(repo)), 2L))
blob_1 <- lookup(repo, tree_1$id[1])
note_8 <- note_create(blob_1, "Note-8")
stopifnot(is(object = lookup(repo, note_8$annotated), class2 = "git_blob"))
stopifnot(identical(length(notes(repo)), 3L))

## Cleanup
unlink(path, recursive = TRUE)
