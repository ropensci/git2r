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

## Create first commit
writeLines("Hello world!", file.path(path, "test-1.txt"))
add(repo, "test-1.txt")
commit_1 <- commit(repo, "First commit message")

## Create and checkout dev branch in repo
checkout(repo, "dev", create = TRUE)

## Create second commit
writeLines(c("Hello world!", "HELLO WORLD!"), file.path(path, "test-2.txt"))
add(repo, "test-2.txt")
commit_2 <- commit(repo, "Second commit message")

## Check files
stopifnot(identical(list.files(path), c("test-1.txt", "test-2.txt")))

## Checkout commit_1 and check files
checkout(commit_1)
stopifnot(identical(list.files(path), "test-1.txt"))

## Checkout commit_2 and check files
checkout(commit_2)
stopifnot(identical(list.files(path), c("test-1.txt", "test-2.txt")))

## Cleanup
unlink(path, recursive = TRUE)
