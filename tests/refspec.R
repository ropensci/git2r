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


## Create directories for repositories in tempdir
path_bare <- tempfile(pattern = "git2r-")
path_repo <- tempfile(pattern = "git2r-")

dir.create(path_bare)
dir.create(path_repo)

## Create bare repository
bare_repo <- init(path_bare, bare = TRUE)

## Clone to repo
repo <- clone(path_bare, path_repo)
config(repo, user.name = "Alice", user.email = "alice@example.org")

## Add changes to repo
writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
           con = file.path(path_repo, "test.txt"))
add(repo, "test.txt")
commit_1 <- commit(repo, "First commit message")

## Add more changes to repo
writeLines(c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
             "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
           con = file.path(path_repo, "test.txt"))
add(repo, "test.txt")
commit_2 <- commit(repo, "Second commit message")

## Check remote
stopifnot(identical(
    git2r:::get_refspec(repo, spec = "master")$remote,
    "origin"))

## Detach
checkout(commit_1)
tools::assertError(git2r:::get_refspec(repo))

## Cleanup
unlink(path_bare, recursive = TRUE)
unlink(path_repo, recursive = TRUE)
