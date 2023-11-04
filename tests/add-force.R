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

library(git2r)
source("util/check.R")

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

## Create a '.gitignore' file
writeLines("test.txt", file.path(path, ".gitignore"))
add(repo, ".gitignore")
commit(repo, "First commit message")

## Create a file
writeLines("Hello world!", file.path(path, "test.txt"))

## Check status
s_1 <- structure(list(staged = empty_named_list(),
                      unstaged = empty_named_list(),
                      untracked = empty_named_list(),
                      ignored = list(ignored = "test.txt")),
                 class = "git_status")
stopifnot(identical(status(repo, ignored = TRUE), s_1))

## The file is ignored and should not be added
add(repo, "test.txt")
stopifnot(identical(status(repo, ignored = TRUE), s_1))

## The file is ignored but should be added with force
s_2 <- structure(list(staged = list(new = "test.txt"),
                      unstaged = empty_named_list(),
                      untracked = empty_named_list(),
                      ignored = empty_named_list()),
                 class = "git_status")

add(repo, "test.txt", force = TRUE)
stopifnot(identical(status(repo, ignored = TRUE), s_2))

## Commit and check status
s_3 <- structure(list(staged = empty_named_list(),
                      unstaged = empty_named_list(),
                      untracked = empty_named_list(),
                      ignored = empty_named_list()),
                 class = "git_status")

commit(repo, "Second commit message")
stopifnot(identical(status(repo, ignored = TRUE), s_3))

## Cleanup
unlink(path, recursive = TRUE)
