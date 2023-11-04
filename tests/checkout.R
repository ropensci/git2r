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

## For debugging
sessionInfo()
libgit2_version()
libgit2_features()


## Create a directory in tempdir
path <- tempfile(pattern = "git2r-")
dir.create(path)

## Initialize a repository
repo <- init(path, branch = "main")
config(repo, user.name = "Alice", user.email = "alice@example.org")

## Create first commit
writeLines("Hello world!", file.path(path, "test.txt"))
add(repo, "test.txt")
commit_1 <- commit(repo, "First commit message")

## Edit file and checkout
writeLines(c("Hello world!", "Hello world!"), file.path(path, "test.txt"))
status_exp_1 <- structure(list(staged = structure(list(),
                                   .Names = character(0)),
                               unstaged = structure(list(modified = "test.txt"),
                                   .Names = "modified"),
                               untracked = structure(list(),
                                   .Names = character(0))),
                          .Names = c("staged", "unstaged", "untracked"),
                          class = "git_status")
status_obs_1 <- status(repo)
str(status_exp_1)
str(status_obs_1)
stopifnot(identical(status_obs_1, status_exp_1))
checkout(repo, path = "test.txt")
status_exp_2 <- structure(list(staged = structure(list(),
                                   .Names = character(0)),
                               unstaged = structure(list(),
                                   .Names = character(0)),
                               untracked = structure(list(),
                                   .Names = character(0))),
                          .Names = c("staged", "unstaged", "untracked"),
                          class = "git_status")
status_obs_2 <- status(repo)
str(status_exp_2)
str(status_obs_2)
stopifnot(identical(status_obs_2, status_exp_2))

## Create second commit
writeLines(c("Hello world!", "HELLO WORLD!"), file.path(path, "test.txt"))
add(repo, "test.txt")
commit_2 <- commit(repo, "Second commit message")
tag(repo, "commit_2", "Tag message")

## Create third commit
writeLines(c("Hello world!", "HELLO WORLD!", "HeLlO wOrLd!"),
           file.path(path, "test.txt"))
add(repo, "test.txt")
commit_3 <- commit(repo, "Third commit message")

## Check HEAD
stopifnot(identical(is_detached(repo), FALSE))
stopifnot(identical(repository_head(repo)$name, "main"))

## Check show and summary
repo
summary(repo)

## Checkout first commit
checkout(commit_1, TRUE)
stopifnot(identical(is_detached(repo), TRUE))
stopifnot(identical(repository_head(repo), commit_1))
stopifnot(identical(readLines(file.path(path, "test.txt")), "Hello world!"))

## Check show and summary
repo
summary(repo)

## Checkout tag
checkout(tags(repo)[[1]], TRUE)
stopifnot(identical(is_detached(repo), TRUE))
stopifnot(identical(readLines(file.path(path, "test.txt")),
                    c("Hello world!", "HELLO WORLD!")))

## Check is_detached with missing repo argument
wd <- setwd(path)
stopifnot(identical(is_detached(), TRUE))
if (!is.null(wd))
    setwd(wd)

## Cleanup
unlink(path, recursive = TRUE)
