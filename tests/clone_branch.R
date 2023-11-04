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


## Create directories for repositories in tempdir
path_bare <- tempfile(pattern = "git2r-")
path_repo_1 <- tempfile(pattern = "git2r-")
path_repo_2 <- tempfile(pattern = "git2r-")

dir.create(path_bare)
dir.create(path_repo_1)
dir.create(path_repo_2)

## Create bare repository
bare_repo <- init(path_bare, bare = TRUE)

## Clone to repo 1
repo_1 <- clone(path_bare, path_repo_1)
config(repo_1, user.name = "Alice", user.email = "alice@example.org")

## Add changes to repo 1
writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
           con = file.path(path_repo_1, "test-1.txt"))
add(repo_1, "test-1.txt")
commit_1 <- commit(repo_1, "First commit message")
branch_name <- branches(repo_1)[[1]]$name

## Create 'dev' branch
checkout(branch_create(commit_1, name = "dev"))

## Add more changes to repo 1
writeLines(c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
             "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
           con = file.path(path_repo_1, "test-1.txt"))
add(repo_1, "test-1.txt")
commit(repo_1, "Second commit message")

## Add more changes to repo 1
writeLines(
    c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
      "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad",
      "minim veniam, quis nostrud exercitation ullamco laboris nisi ut"),
    con = file.path(path_repo_1, "test-1.txt"))
add(repo_1, "test-1.txt")
commit(repo_1, "Third commit message")

## Push to bare
push(repo_1, "origin", paste0("refs/heads/", branch_name))
push(repo_1, "origin", "refs/heads/dev")

## Print branch
branches(repo_1)[[paste0("origin/", branch_name)]]

## Clone to repo 2
repo_2 <- clone(url = path_bare, local_path = path_repo_2, branch = "dev")
config(repo_2, user.name = "Bob", user.email = "bob@example.org")

## Check branch and commits
stopifnot(identical(length(commits(repo_2)), 3L))
stopifnot(identical(repository_head(repo_2)$name, "dev"))

## Cleanup
unlink(path_bare, recursive = TRUE)
unlink(path_repo_1, recursive = TRUE)
unlink(path_repo_2, recursive = TRUE)
