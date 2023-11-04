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
setwd(path)

## Initialize a repository
repo <- init(path)
config(repo, user.name = "Alice", user.email = "alice@example.org")

## Test to add file with a leading './'
writeLines("foo-1", file.path(path, "foo-1"))
add(repo, "./foo-1")
status_exp <- structure(list(staged = list(new = "foo-1"),
                             unstaged = empty_named_list(),
                             untracked = empty_named_list()),
                        class = "git_status")
status_obs <- status(repo)
str(status_exp)
str(status_obs)
stopifnot(identical(status_obs, status_exp))

## Test to add file in sub-folder with sub-folder as working directory
writeLines("foo-2", file.path(path, "foo-2"))
dir.create(file.path(path, "foo_dir"))
writeLines("foo-2", file.path(path, "foo_dir/foo-2"))
setwd("./foo_dir")
add(repo, "foo-2")
status_exp <- structure(list(staged = list(new = "foo-1",
                                           new = "foo_dir/foo-2"),
                             unstaged = empty_named_list(),
                             untracked = list(untracked = "foo-2")),
                        class = "git_status")
status_obs <- status(repo)
str(status_exp)
str(status_obs)
stopifnot(identical(status_obs, status_exp))

## Test glob expansion
setwd(tempdir())
dir.create(file.path(path, "glob_dir"))
writeLines("a", file.path(path, "glob_dir/a.txt"))
writeLines("b", file.path(path, "glob_dir/b.txt"))
writeLines("c", file.path(path, "glob_dir/c.txt"))
writeLines("d", file.path(path, "glob_dir/d.md"))
add(repo, "glob_dir/*txt")
status_exp <- structure(list(staged = list(new = "foo-1",
                                           new = "foo_dir/foo-2",
                                           new = "glob_dir/a.txt",
                                           new = "glob_dir/b.txt",
                                           new = "glob_dir/c.txt"),
                             unstaged = empty_named_list(),
                             untracked = list(untracked = "foo-2",
                                              untracked = "glob_dir/d.md")),
                        class = "git_status")
status_obs <- status(repo)
str(status_exp)
str(status_obs)
stopifnot(identical(status_obs, status_exp))

## Test glob expansion with relative path
setwd(path)
add(repo, "./glob_dir/*md")
status_exp <- structure(list(staged = list(new = "foo-1",
                                           new = "foo_dir/foo-2",
                                           new = "glob_dir/a.txt",
                                           new = "glob_dir/b.txt",
                                           new = "glob_dir/c.txt",
                                           new = "glob_dir/d.md"),
                             unstaged = empty_named_list(),
                             untracked = list(untracked = "foo-2")),
                        class = "git_status")
status_obs <- status(repo)
str(status_exp)
str(status_obs)
stopifnot(identical(status_obs, status_exp))

## Test to add file in root of workdir when the file also exists in
## current workdir.
setwd(tempdir())
writeLines("e", file.path(path, "e.txt"))
writeLines("e", file.path(tempdir(), "e.txt"))
add(repo, "e.txt")
status_exp <- structure(list(staged = list(new = "e.txt",
                                           new = "foo-1",
                                           new = "foo_dir/foo-2",
                                           new = "glob_dir/a.txt",
                                           new = "glob_dir/b.txt",
                                           new = "glob_dir/c.txt",
                                           new = "glob_dir/d.md"),
                             unstaged = empty_named_list(),
                             untracked = list(untracked = "foo-2")),
                        class = "git_status")
status_obs <- status(repo)
str(status_exp)
str(status_obs)
stopifnot(identical(status_obs, status_exp))

## Cleanup
unlink(path, recursive = TRUE)
