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

library(git2r)

##
## Create a directory in tempdir
##
path <- tempfile(pattern="git2r-")
dir.create(path)

##
## Initialize a repository
##
repo <- init(path)
config(repo, user.name="Repo", user.email="repo@example.org")

##
## Create a file, add and commit
##
writeLines("Hello world!", file.path(path, "test.txt"))
add(repo, "test.txt")
commit.1 <- commit(repo, "Commit message 1")

##
## Create another commit
##
writeLines(c("Hello world!",
             "HELLO WORLD!"),
           file.path(path, "test.txt"))
add(repo, "test.txt")
commit.2 <- commit(repo, "Commit message 2")

##
## Check default ref
##
stopifnot(identical(note_default_ref(repo),
                    "refs/notes/commits"))

##
## Check that note_list is an empty list
##
stopifnot(identical(note_list(repo), list()))

##
## Create note in default namespace
##
note.1 <- note_create(commit.1, "Note-1")
stopifnot(identical(length(note_list(repo)), 1L))
tools::assertError(note_create(commit.1, "Note-2"))
note.2 <- note_create(commit.1, "Note-2", force = TRUE)
stopifnot(identical(length(note_list(repo)), 1L))

##
## Create note in named (review) namespace
##
note.3 <- note_create(commit.1, "Note-3", ref="ref/notes/review")
note.4 <- note_create(commit.2, "Note-4", ref="ref/notes/review")
stopifnot(identical(length(note_list(repo, ref="ref/notes/review")), 2L))

##
## Cleanup
##
unlink(path, recursive=TRUE)
