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

## Create a directory in tempdir
path <- tempfile(pattern="git2r-")
dir.create(path)

## Initialize a repository
repo <- init(path)
config(repo, user.name="Developer", user.email="developer@example.org")

## Create a file and commit
writeLines("Hello world!", file.path(path, "test.txt"))
add(repo, "test.txt")
commit_1 <- commit(repo, "First commit message")

## Change file and commit
writeLines(c("Hello world!", "HELLO WORLD!"),
           file.path(path, "test.txt"))
add(repo, "test.txt")
commit_2 <- commit(repo, "Second commit message")

## Check ahead behind
stopifnot(identical(ahead_behind(commit_1, commit_2), c(1L, 0L)))

## Cleanup
unlink(path, recursive=TRUE)
