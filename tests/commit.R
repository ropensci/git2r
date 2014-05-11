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
## Create a directory in tempdir
##
path <- tempfile(pattern="git2r-")
dir.create(path)

##
## Initialize a repository
##
repo <- init(path)
config(repo, user.name="Repo", user.email="repo@example.org")

##
## Commit without adding changes should produce an error
##
tools::assertError(commit(repo, "Test to commit"))

##
## Create a file
##
writeLines("Hello world!", file.path(path, "test.txt"))

##
## Commit without adding changes should produce an error
##
tools::assertError(commit(repo, "Test to commit"))

##
## add and commit
##
add(repo, "test.txt")
commit_1 <- commit(repo, "Commit message")

##
## Check commit
##
stopifnot(identical(commit_1@author@name, "Repo"))
stopifnot(identical(commit_1@author@email, "repo@example.org"))
stopifnot(identical(lookup(repo, commit_1@hex), commit_1))
stopifnot(identical(length(commits(repo)), 1L))
stopifnot(identical(commits(repo)[[1]]@author@name, "Repo"))
stopifnot(identical(commits(repo)[[1]]@author@email, "repo@example.org"))
stopifnot(is.null(parents(commit_1)))

##
## Commit without adding changes should produce an error
##
tools::assertError(commit(repo, "Test to commit"))

##
## Add another commit
##
writeLines(c("Hello world!", "HELLO WORLD!"), file.path(path, "test.txt"))
add(repo, "test.txt")
commit_2 <- commit(repo, "Commit message 2")

##
## Check relationship
##
stopifnot(identical(descendant_of(commit_2, commit_1), TRUE))
stopifnot(identical(descendant_of(commit_1, commit_2), FALSE))
stopifnot(identical(parents(commit_2), commit_1))

##
## Cleanup
##
unlink(path, recursive=TRUE)
