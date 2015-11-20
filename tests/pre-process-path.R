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

## Create a directory in tempdir
path <- tempfile(pattern="git2r-")
dir.create(path)
setwd(path)

## Initialize a repository
repo <- init(path)
config(repo, user.name="Alice", user.email="alice@example.org")

## Test to add file with a leading './'
writeLines("foo-1", file.path(path, "foo-1"))
add(repo, "./foo-1")
status_exp_1 <- structure(list(staged = structure(list(new = "foo-1"),
                                   .Names = "new"),
                               unstaged = structure(list(),
                                   .Names = character(0)),
                               untracked = structure(list(),
                                   .Names = character(0))),
                          .Names = c("staged", "unstaged", "untracked"),
                          class = "git_status")
status_obs_1 <- status(repo)
str(status_exp_1)
str(status_obs_1)
stopifnot(identical(status_obs_1, status_exp_1))

## Test to add file in sub-folder with sub-folder as working directory
writeLines("foo-2", file.path(path, "foo-2"))
dir.create(file.path(path, "foo_dir"))
writeLines("foo-2", file.path(path, "foo_dir/foo-2"))
setwd("./foo_dir")
add(repo, "foo-2")
status_exp_2 <- structure(list(staged = structure(list(new = "foo-1",
                                   new = "foo_dir/foo-2"),
                                   .Names = c("new", "new")),
                               unstaged = structure(list(),
                                   .Names = character(0)),
                               untracked = structure(list(untracked = "foo-2"),
                                   .Names = "untracked")),
                          .Names = c("staged", "unstaged", "untracked"),
                          class = "git_status")
status_obs_2 <- status(repo)
str(status_exp_2)
str(status_obs_2)
stopifnot(identical(status_obs_2, status_exp_2))

## Cleanup
unlink(path, recursive = TRUE)
