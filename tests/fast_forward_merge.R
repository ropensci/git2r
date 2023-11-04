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


## Create directory for repository in tempdir
path <- tempfile(pattern = "git2r-")
dir.create(path)

## Create repository
repo <- init(path, branch = "main")
config(repo, user.name = "Alice", user.email = "alice@example.org")

## Add changes to repo
writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
           con = file.path(path, "test-1.txt"))
add(repo, "test-1.txt")
commit_1 <- commit(repo, "First commit message")

## Create branch and checkout
checkout(branch_create(commit_1, name = "test"))

## Add changes to test branch
writeLines(c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
             "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
           con = file.path(path, "test-1.txt"))
add(repo, "test-1.txt")
commit_2 <- commit(repo, "Second commit message")

# Checkout main and merge
b <- branches(repo)
checkout(b[sapply(b, "[", "name") == "main"][[1]], force = TRUE)
m <- merge(b[sapply(b, "[", "name") == "test"][[1]])

# Check merge
stopifnot(inherits(m, "git_merge_result"))
stopifnot(identical(m$up_to_date, FALSE))
stopifnot(identical(m$fast_forward, TRUE))
stopifnot(identical(m$conflicts, FALSE))
stopifnot(identical(sha(m), NA_character_))
stopifnot(identical(length(commits(repo)), 2L))

# Check reflog
r <- reflog(repo)
stopifnot(identical(r[[1]]$message, "merge test: Fast-forward"))

## Cleanup
unlink(path, recursive = TRUE)
