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

## Create a file and commit
writeLines("Hello world!", file.path(path, "test.txt"))
add(repo, "test.txt")
commit_1 <- commit(repo, "First commit message")
tag_1 <- tag(repo, "Tagname1", "Tag message 1")

## Change file and commit
writeLines(c("Hello world!", "HELLO WORLD!"),
           file.path(path, "test.txt"))
add(repo, "test.txt")
commit_2 <- commit(repo, "Second commit message")
tag_2 <- tag(repo, "Tagname2", "Tag message 2")

## Check ahead behind
stopifnot(identical(ahead_behind(commit_1, commit_2), c(0L, 1L)))
stopifnot(identical(ahead_behind(tag_1, tag_2), c(0L, 1L)))
stopifnot(identical(ahead_behind(tag_2, tag_1), c(1L, 0L)))
stopifnot(identical(ahead_behind(commit_1, branches(repo)[[1]]), c(0L, 1L)))
stopifnot(identical(ahead_behind(branches(repo)[[1]], commit_1), c(1L, 0L)))

## Cleanup
unlink(path, recursive = TRUE)
