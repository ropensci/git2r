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


## Create 2 directories in tempdir
path_src <- tempfile(pattern = "git2r-")
path_tgt <- tempfile(pattern = "git2r-")
dir.create(path_tgt)
dir.create(path_src)

## Initialize a repository
repo_src <- init(path_src)
config(repo_src, user.name = "Alice", user.email = "alice@example.org")

## Add commit to repo
filename <- "test.txt"
writeLines("Hello world", con = file.path(path_src, filename))
add(repo_src, "test.txt")
commit_src <- commit(repo_src, "Commit message")

## Check checkout argument
tools::assertError(clone(path_src, path_tgt, checkout = c(FALSE, TRUE)))
tools::assertError(clone(path_src, path_tgt, checkout = 1))
tools::assertError(clone(path_src, path_tgt, checkout = 1L))
tools::assertError(clone(path_src, path_tgt, checkout = "test"))

## Clone source to target repository without checking out any files
repo_tgt <- clone(path_src, path_tgt, checkout = FALSE)

## List files in the repositores
stopifnot(identical(list.files(path_src), filename))
stopifnot(identical(list.files(path_tgt), character(0)))

## Compare commits
stopifnot(identical(length(commits(repo_tgt)), 1L))
commit_tgt <- last_commit(repo_tgt)
stopifnot(identical(sha(last_commit(path_tgt)), sha(commit_tgt)))
stopifnot(identical(sha(commit_src), sha(commit_tgt)))
stopifnot(identical(commit_src$author, commit_tgt$author))
stopifnot(identical(commit_src$committer, commit_tgt$committer))
stopifnot(identical(commit_src$summary, commit_tgt$summary))
stopifnot(identical(commit_src$message, commit_tgt$message))
stopifnot(!identical(commit_src$repo, commit_tgt$repo))

## Cleanup
unlink(path_tgt, recursive = TRUE)
unlink(path_src, recursive = TRUE)
