## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2014 The git2r contributors
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

## Create directories for repositories in tempdir
path_bare <- tempfile(pattern="git2r-")
path_repo_1 <- tempfile(pattern="git2r-")
path_repo_2 <- tempfile(pattern="git2r-")

dir.create(path_bare)
dir.create(path_repo_1)
dir.create(path_repo_2)

## Create bare repository
bare_repo <- init(path_bare, bare = TRUE)

## Clone to repo 1
repo_1 <- clone(path_bare, path_repo_1)
config(repo_1, user.name="Repo One", user.email="repo.one@example.org")

## Add changes to repo 1 and push to bare
writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
           con = file.path(path_repo_1, "test-1.txt"))
add(repo_1, "test-1.txt")
commit_1 <- commit(repo_1, "First commit message")
push(repo_1, "origin", "refs/heads/master")

## Clone to repo 2
repo_2 <- clone(path_bare, path_repo_2)
config(repo_2, user.name="Repo Two", user.email="repo.two@example.org")

## Add more changes to repo 1 and push to bare
writeLines(c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
             "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
           con = file.path(path_repo_1, "test-1.txt"))
add(repo_1, "test-1.txt")
commit_2 <- commit(repo_1, "Second commit message")
push(repo_1, "origin", "refs/heads/master")

## Pull changes to repo_2
pull(repo_2)
stopifnot(identical(length(commits(repo_2)), 2L))

## Check remote url of repo_2
stopifnot(identical(branch_remote_url(branch_get_upstream(head(repo_2))),
                    path_bare))

## Unset remote remote tracking branch
branch_set_upstream(head(repo_2), NULL)
stopifnot(is.null(branch_get_upstream(head(repo_2))))
tools::assertError(pull(repo_2))
tools::assertError(branch_set_upstream(head(repo_2), NULL))

## Add more changes to repo 1 and push to bare
writeLines(c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
             "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad",
             "minim veniam, quis nostrud exercitation ullamco laboris nisi ut"),
           con = file.path(path_repo_1, "test-1.txt"))
add(repo_1, "test-1.txt")
commit(repo_1, "Third commit message")
push(repo_1)

## Set remote tracking branch
branch_set_upstream(head(repo_2), "origin/master")
stopifnot(identical(branch_remote_url(branch_get_upstream(head(repo_2))),
                    path_bare))

## Pull changes to repo_2
pull(repo_2)
stopifnot(identical(length(commits(repo_2)), 3L))

## Cleanup
unlink(path_bare, recursive=TRUE)
unlink(path_repo_1, recursive=TRUE)
unlink(path_repo_2, recursive=TRUE)
