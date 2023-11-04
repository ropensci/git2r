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

library("git2r")

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

## Check that reflog is empty
stopifnot(identical(reflog(repo), structure(list(), class = "git_reflog")))

## Create a file
writeLines("Hello world!", file.path(path, "test.txt"))

## add and commit
add(repo, "test.txt")
commit_1 <- commit(repo, "Commit message")

## Check that reflog is not empry
stopifnot(identical(length(reflog(repo)), 1L))
reflog_entry <- reflog(repo)[[1]]
stopifnot(identical(sha(reflog_entry), sha(commit_1)))
stopifnot(identical(reflog_entry$refname, "HEAD"))
stopifnot(identical(reflog_entry$index, 0L))
stopifnot(identical(reflog_entry$committer$email, "alice@example.org"))
stopifnot(identical(reflog_entry$message, "commit (initial): Commit message"))

## Check printing
r <- reflog(repo)
stopifnot(identical(print(r), r))

## Cleanup
unlink(path, recursive = TRUE)
