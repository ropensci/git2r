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

##
## Create 2 directories in tempdir
##
path_bare <- tempfile(pattern="git2r-")
path_repo_1 <- tempfile(pattern="git2r-")
path_repo_2 <- tempfile(pattern="git2r-")

dir.create(path_bare)
dir.create(path_repo_1)
dir.create(path_repo_2)

##
## Create repositories
##
bare_repo <- init(path_bare, bare = TRUE)
repo_1 <- clone(path_bare, path_repo_1)
repo_2 <- clone(path_bare, path_repo_2)

##
## Config repositories
##
config(repo_1, user.name="Repo One", user.email="repo.one@example.org")
config(repo_2, user.name="Repo Two", user.email="repo.two@example.org")

##
## Add changes to repo 1
##
writeLines("Hello world", con = file.path(path_repo_1, "test.txt"))
add(repo_1, "test.txt")
commit_1 <- commit(repo_1, "Commit message")

##
## Push changes from repo 1 to origin
##
push(repo_1, "origin", "refs/heads/master")

##
## Check result in bare repository
##
stopifnot(identical(length(commits(bare_repo)), 1L))
bare_commit_1 <- commits(bare_repo)[[1]]
stopifnot(identical(commit_1@sha, bare_commit_1@sha))
stopifnot(identical(commit_1@author, bare_commit_1@author))
stopifnot(identical(commit_1@committer, bare_commit_1@committer))
stopifnot(identical(commit_1@summary, bare_commit_1@summary))
stopifnot(identical(commit_1@message, bare_commit_1@message))
stopifnot(!identical(commit_1@repo, bare_commit_1@repo))

##
## Fetch
##
fetch(repo_2, "origin")

##
## Test show method of non-empty repository where head is null
##
show(repo_2)

##
## Cleanup
##
unlink(path_bare, recursive=TRUE)
unlink(path_repo_1, recursive=TRUE)
unlink(path_repo_2, recursive=TRUE)
