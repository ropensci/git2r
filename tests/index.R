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

## Create directories
dir.create(file.path(path, "sub-folder"));
dir.create(file.path(path, "sub-folder", "sub-sub-folder"));

## Create files
writeLines("Hello world!",
           file.path(path, "file-1.txt"))
writeLines("Hello world!",
           file.path(path, "sub-folder", "file-2.txt"))
writeLines("Hello world!",
           file.path(path, "sub-folder", "file-3.txt"))
writeLines("Hello world!",
           file.path(path, "sub-folder", "sub-sub-folder", "file-4.txt"))
writeLines("Hello world!",
           file.path(path, "sub-folder", "sub-sub-folder", "file-5.txt"))

## Add
add(repo, "file-1.txt")
status_exp <- structure(list(staged = list(new = "file-1.txt"),
                             unstaged = empty_named_list(),
                             untracked = list(untracked = "sub-folder/")),
                        class = "git_status")
status_obs <- status(repo)
stopifnot(identical(status_obs, status_exp))

## Index remove by path
index_remove_bypath(repo, "file-1.txt")
status_exp <- structure(list(staged = empty_named_list(),
                             unstaged = empty_named_list(),
                             untracked = list(untracked = "file-1.txt",
                                              untracked = "sub-folder/")),
                        class = "git_status")
status_obs <- status(repo)
stopifnot(identical(status_obs, status_exp))

## Add
add(repo, "sub-folder")
status_exp <- structure(list(staged = list(
                                 new = "sub-folder/file-2.txt",
                                 new = "sub-folder/file-3.txt",
                                 new = "sub-folder/sub-sub-folder/file-4.txt",
                                 new = "sub-folder/sub-sub-folder/file-5.txt"),
                             unstaged = empty_named_list(),
                             untracked = list(untracked = "file-1.txt")),
                        class = "git_status")
status_obs <- status(repo)
stopifnot(identical(status_obs, status_exp))

## Commit
commit(repo, "First commit message")

## It should fail to remove non-existing, untracked and ignored files
tools::assertError(rm_file(repo, c("file-1.txt", "file-2.txt")))
tools::assertError(rm_file(repo, c("file-1.txt", "")))
tools::assertError(rm_file(repo, c("file-1.txt")))
writeLines("/file-1.txt", file.path(path, ".gitignore"))
tools::assertError(rm_file(repo, "file-1.txt"))

## It should fail to remove files with staged changes
file.remove(file.path(path, ".gitignore"))
add(repo, "file-1.txt")
tools::assertError(rm_file(repo, "file-1.txt"))

## It should fail to remove files with unstaged changes
commit(repo, "Second commit message")
writeLines(c("Hello world!", "Hello world!"),
           file.path(path, "file-1.txt"))
tools::assertError(rm_file(repo, "file-1.txt"))

## Remove file
add(repo, "file-1.txt")
commit(repo, "Third commit message")
rm_file(repo, "file-1.txt")
status_exp <- structure(list(staged = list(deleted = "file-1.txt"),
                             unstaged = empty_named_list(),
                             untracked = empty_named_list()),
                        class = "git_status")
status_obs <- status(repo)
stopifnot(identical(status_obs, status_exp))

## Cleanup
unlink(path, recursive = TRUE)
