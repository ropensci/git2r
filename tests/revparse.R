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

## Create a file
writeLines("Hello world!", file.path(path, "test.txt"))

## Add and commit
add(repo, "test.txt")
commit_1 <- commit(repo, "First commit message")

tools::assertError(revparse_single(repo, "HEAD^"))

## Add another commit
writeLines(c("Hello world!", "HELLO WORLD!"), file.path(path, "test.txt"))
add(repo, "test.txt")
commit(repo, "Second commit message")

stopifnot(identical(revparse_single(repo, "HEAD^"), commit_1))
stopifnot(is_blob(revparse_single(repo, "HEAD:test.txt")))

## Cleanup
unlink(path, recursive = TRUE)
