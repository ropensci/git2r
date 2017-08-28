## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2015 The git2r contributors
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

## For debugging
sessionInfo()

## Create a directory in tempdir
path <- tempfile(pattern="git2r-")
dir.create(path)

## Initialize a repository
repo <- init(path)
config(repo, user.name="Alice", user.email="alice@example.org")

## Create a '.gitignore' file
writeLines("test.txt", file.path(path, ".gitignore"))
add(repo, ".gitignore")
commit(repo, "First commit message")

## Create a file
writeLines("Hello world!", file.path(path, "test.txt"))

## Check status
s_1 <- structure(list(staged = structure(list(), .Names = character(0)),
                      unstaged = structure(list(), .Names = character(0)),
                      untracked = structure(list(), .Names = character(0)),
                      ignored = structure(list(ignored = "test.txt"),
                          .Names = "ignored")),
                 .Names = c("staged", "unstaged", "untracked", "ignored"),
                 class = "git_status")
stopifnot(identical(status(repo, ignored = TRUE), s_1))

## The file is ignored and should not be added
add(repo, "test.txt")
stopifnot(identical(status(repo, ignored = TRUE), s_1))

## The file is ignored but should be added with force
s_2 <- structure(list(staged = structure(list(new = "test.txt"), .Names = "new"),
                      unstaged = structure(list(), .Names = character(0)),
                      untracked = structure(list(), .Names = character(0)),
                      ignored = structure(list(), .Names = character(0))),
                 .Names = c("staged", "unstaged", "untracked", "ignored"),
                 class = "git_status")

add(repo, "test.txt", force = TRUE)
stopifnot(identical(status(repo, ignored = TRUE), s_2))

## Commit and check status
s_3 <- structure(list(staged = structure(list(), .Names = character(0)),
                      unstaged = structure(list(), .Names = character(0)),
                      untracked = structure(list(), .Names = character(0)),
                      ignored = structure(list(), .Names = character(0))),
                 .Names = c("staged", "unstaged", "untracked", "ignored"),
                 class = "git_status")

commit(repo, "Second commit message")
stopifnot(identical(status(repo, ignored = TRUE), s_3))

## Ignore a subdirectory
writeLines("subdir", file.path(path, ".gitignore"))

add(repo, ".gitignore")
commit(repo, "Ignore subdir")

## Create subdirectory and a file inside
d_ignore <- file.path(path, "subdir")
dir.create(d_ignore)

writeLines("An exception", file.path(d_ignore, "force.txt"))

s_4 <- structure(list(staged = structure(list(), .Names = character(0)),
                      unstaged = structure(list(), .Names = character(0)),
                      untracked = structure(list(), .Names = character(0)),
                      ignored = structure(list(ignored = "subdir/"), .Names = "ignored")),
                 .Names = c("staged", "unstaged", "untracked", "ignored"),
                 class = "git_status")
stopifnot(identical(status(repo, ignored = TRUE), s_4))

## The file is ignored and should not be added
add(repo, "subdir/force.txt")
stopifnot(identical(status(repo, ignored = TRUE), s_4))

## The file is ignored but should be added with force
add(repo, "subdir/force.txt", force = TRUE)

s_5 <- structure(list(staged = structure(list(new = "subdir/test.txt"), .Names = "new"),
                      unstaged = structure(list(), .Names = character(0)),
                      untracked = structure(list(), .Names = character(0)),
                      ignored = structure(list(ignored = "subdir/"), .Names = "ignored")),
                 .Names = c("staged", "unstaged", "untracked", "ignored"),
                 class = "git_status")
stopifnot(identical(status(repo, ignored = TRUE), s_5))

## Commit and check status
commit(repo, "Commit file in ignored subdirectory")
stopifnot(identical(status(repo, ignored = TRUE), s_4))

## Cleanup
unlink(path, recursive=TRUE)
