## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2018 The git2r contributors
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

## Create a directory in tempdir
path <- tempfile(pattern="git2r-")
dir.create(path)

## Initialize a repository
repo <- init(path)
config(repo, user.name="Alice", user.email="alice@example.org")

## Create a file, add and commit
writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
           con = file.path(path, "test.txt"))
add(repo, "test.txt")
commit_1 <- commit(repo, "Commit message 1")

## Create branch, checkout, change file and commit
b <- checkout(repo, branch = "branch", create = TRUE)
writeLines(c("Lorem ipsum dolor amet sit, consectetur adipisicing elit, sed do",
             "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
           con = file.path(path, "test.txt"))
add(repo, "test.txt")
commit(repo, "Commit message branch")

## Checkout master and create a conflicting change
b <- branches(repo)
checkout(b[sapply(b, "[", "name") == "master"][[1]], force=TRUE)
writeLines(c("Lorem ipsum dolor sit amet, adipisicing consectetur elit, sed do",
             "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
           con = file.path(path, "test.txt"))

## Attempt to merge with unstaged conflicting change
status_unstaged <- structure(list(
    staged = structure(list(), .Names = character(0)),
    unstaged = structure(list(modified = "test.txt"), .Names = "modified"),
    untracked = structure(list(), .Names = character(0))),
    .Names = c("staged", "unstaged", "untracked"),
    class = "git_status")
stopifnot(identical(status(repo), status_unstaged))
m <- merge(repo, "branch")
stopifnot(identical(m$up_to_date, FALSE))
stopifnot(identical(m$fast_forward, FALSE))
stopifnot(identical(m$conflicts, TRUE))
stopifnot(identical(sha(m), NA_character_))
stopifnot(identical(format(m), "Merge: Conflicts"))

## Cleanup
unlink(path, recursive=TRUE)
