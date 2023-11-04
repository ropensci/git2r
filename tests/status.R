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

## Status case 1
status_exp_1 <- structure(list(staged = empty_named_list(),
                               unstaged = empty_named_list(),
                               untracked = empty_named_list()),
                          class = "git_status")
status_obs_1 <- status(repo)
stopifnot(identical(print(status_obs_1), status_obs_1))
str(status_exp_1)
str(status_obs_1)
stopifnot(identical(status_obs_1, status_exp_1))
stopifnot(identical(capture.output(status(repo)),
                    "working directory clean"))

## Status case 2, include ignored files
status_exp_2 <- structure(list(staged = empty_named_list(),
                               unstaged = empty_named_list(),
                               untracked = empty_named_list(),
                               ignored = empty_named_list()),
                          class = "git_status")
status_obs_2 <- status(repo, ignored = TRUE)
status_obs_2
str(status_exp_2)
str(status_obs_2)
stopifnot(identical(status_obs_2, status_exp_2))
stopifnot(identical(capture.output(status(repo, ignored = TRUE)),
                    "working directory clean"))

## Create 4 files
writeLines("File-1", file.path(path, "test-1.txt"))
writeLines("File-2", file.path(path, "test-2.txt"))
writeLines("File-3", file.path(path, "test-3.txt"))
writeLines("File-4", file.path(path, "test-4.txt"))

## Status case 3: 4 untracked files
status_exp_3 <- structure(list(staged = empty_named_list(),
                               unstaged = empty_named_list(),
                               untracked = list(untracked = "test-1.txt",
                                                untracked = "test-2.txt",
                                                untracked = "test-3.txt",
                                                untracked = "test-4.txt")),
                          class = "git_status")
status_obs_3 <- status(repo)
status_obs_3
str(status_exp_3)
str(status_obs_3)
stopifnot(identical(status_obs_3, status_exp_3))

## Add file 1 and 2 to the repository and commit
add(repo, c("test-1.txt", "test-2.txt"))
commit(repo, "Commit message")

## Status case 4: 2 untracked files
status_exp_4 <- structure(list(staged = empty_named_list(),
                               unstaged = empty_named_list(),
                               untracked = list(untracked = "test-3.txt",
                                                untracked = "test-4.txt")),
                          class = "git_status")
status_obs_4 <- status(repo)
status_obs_4
str(status_exp_4)
str(status_obs_4)
stopifnot(identical(status_obs_4, status_exp_4))

## Update file 1 & 2
writeLines(c("File-1", "Hello world"), file.path(path, "test-1.txt"))
writeLines(c("File-2", "Hello world"), file.path(path, "test-2.txt"))

## Add file 1
add(repo, "test-1.txt")

## Status case 5: 1 staged file, 1 unstaged file and 2 untracked files
status_exp_5 <- structure(list(staged = list(modified = "test-1.txt"),
                               unstaged = list(modified = "test-2.txt"),
                               untracked = list(untracked = "test-3.txt",
                                                untracked = "test-4.txt")),
                          class = "git_status")
status_obs_5 <- status(repo)
status_obs_5
str(status_exp_5)
str(status_obs_5)
stopifnot(identical(status_obs_5, status_exp_5))

## Add .gitignore file with file test-4.txt
writeLines("test-4.txt", file.path(path, ".gitignore"))

## Status case 6: 1 staged file, 1 unstaged file, 2 untracked files
## and 1 ignored file
status_exp_6 <- structure(list(staged = list(modified = "test-1.txt"),
                               unstaged = list(modified = "test-2.txt"),
                               untracked = list(untracked = ".gitignore",
                                                untracked = "test-3.txt"),
                               ignored = list(ignored = "test-4.txt")),
                          class = "git_status")
status_obs_6 <- status(repo, ignore = TRUE)
status_obs_6
str(status_exp_6)
str(status_obs_6)
stopifnot(identical(status_obs_6, status_exp_6))

## Cleanup
unlink(path, recursive = TRUE)
