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
path_bare <- tempfile(pattern = "git2r-")
path_repo <- tempfile(pattern = "git2r-")
dir.create(path_bare)
dir.create(path_repo)

## Initialize a repository
repo <- init(path_repo)
config(repo, user.name = "Alice", user.email = "alice@example.org")

## Add commit to repo
writeLines("Hello world", con = file.path(path_repo, "test.txt"))
add(repo, "test.txt")
commit_1 <- commit(repo, "Commit message")

## Check bare argument
tools::assertError(clone(path_repo, path_bare, bare = c(TRUE, TRUE)))
tools::assertError(clone(path_repo, path_bare, bare = 1))
tools::assertError(clone(path_repo, path_bare, bare = 1L))
tools::assertError(clone(path_repo, path_bare, bare = "test"))

## Clone repo to bare repository
bare_repo <- clone(path_repo, path_bare, bare = TRUE)

## Check the repositores
stopifnot(identical(is_bare(bare_repo), TRUE))
stopifnot(identical(is_bare(repo), FALSE))

## Check result in bare repository
stopifnot(identical(length(commits(bare_repo)), 1L))
bare_commit_1 <- commits(bare_repo)[[1]]
stopifnot(identical(sha(commit_1), sha(bare_commit_1)))
stopifnot(identical(commit_1$author, bare_commit_1$author))
stopifnot(identical(commit_1$committer, bare_commit_1$committer))
stopifnot(identical(commit_1$summary, bare_commit_1$summary))
stopifnot(identical(commit_1$message, bare_commit_1$message))
stopifnot(!identical(commit_1$repo, bare_commit_1$repo))

## Cleanup
unlink(path_bare, recursive = TRUE)
unlink(path_repo, recursive = TRUE)
