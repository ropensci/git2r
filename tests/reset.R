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

## Create a file
writeLines("Hello world!", file.path(path, "test-1.txt"))

## Add and reset an empty repository using a path
add(repo, "test-1.txt")
stopifnot(identical(
    status(repo),
    structure(list(staged = list(new = "test-1.txt"),
                   unstaged = empty_named_list(),
                   untracked = empty_named_list()),
              class = "git_status")))
reset(repo, path = "test-1.txt")
stopifnot(identical(
    status(repo),
    structure(list(staged = empty_named_list(),
                   unstaged = empty_named_list(),
                   untracked = list(untracked = "test-1.txt")),
              class = "git_status")))

## Add and reset a non-empty repository using a path
add(repo, "test-1.txt")
commit(repo, "First commit")
writeLines(c("Hello world!", "HELLO WORLD!"), file.path(path, "test-1.txt"))
add(repo, "test-1.txt")
stopifnot(identical(
    status(repo),
    structure(list(staged = list(modified = "test-1.txt"),
                   unstaged = empty_named_list(),
                   untracked = empty_named_list()),
              class = "git_status")))
reset(repo, path = "test-1.txt")
stopifnot(identical(
    status(repo),
    structure(list(staged = empty_named_list(),
                   unstaged = list(modified = "test-1.txt"),
                   untracked = empty_named_list()),
              class = "git_status")))

## add and commit
add(repo, "test-1.txt")
commit_1 <- commit(repo, "Commit message")

## Make one more commit
writeLines(c("Hello world!", "HELLO WORLD!", "hello world!"),
           file.path(path, "test-1.txt"))
add(repo, "test-1.txt")
commit(repo, "Next commit message")

## Create one more file
writeLines("Hello world!", file.path(path, "test-2.txt"))

## 'soft' reset to first commit
reset(commit_1)
soft_exp <- structure(list(staged = list(modified = "test-1.txt"),
                           unstaged = empty_named_list(),
                           untracked = list(untracked = "test-2.txt")),
                      class = "git_status")
soft_obs <- status(repo)
stopifnot(identical(soft_obs, soft_exp))
stopifnot(identical(length(commits(repo)), 2L))
stopifnot(identical(commits(repo)[[1]], commit_1))

## 'mixed' reset to first commit
commit(repo, "Next commit message")
reset(commit_1, "mixed")
mixed_exp <- structure(list(staged = empty_named_list(),
                            unstaged = list(modified = "test-1.txt"),
                            untracked = list(untracked = "test-2.txt")),
                       class = "git_status")
mixed_obs <- status(repo)
stopifnot(identical(mixed_obs, mixed_exp))
stopifnot(identical(length(commits(repo)), 2L))
stopifnot(identical(commits(repo)[[1]], commit_1))

## 'hard' reset to first commit
add(repo, "test-1.txt")
commit(repo, "Next commit message")
reset(commit_1, "hard")
hard_exp <- structure(list(staged = empty_named_list(),
                           unstaged = empty_named_list(),
                           untracked = list(untracked = "test-2.txt")),
                      class = "git_status")
hard_obs <- status(repo)
stopifnot(identical(hard_obs, hard_exp))
stopifnot(identical(length(commits(repo)), 2L))
stopifnot(identical(commits(repo)[[1]], commit_1))

## Cleanup
unlink(path, recursive = TRUE)
