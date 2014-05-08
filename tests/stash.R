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
## Create a file
##
writeLines("Hello world!", file.path(path, "test-1.txt"))

##
## add and commit
##
add(repo, 'test-1.txt')
commit(repo, "Commit message")

##
## Make one more commit
##
writeLines(c("Hello world!", "HELLO WORLD!"), file.path(path, "test-1.txt"))
add(repo, 'test-1.txt')
commit(repo, "Next commit message")

##
## Check that there are no stashes
##
stopifnot(identical(stashes(repo), list()))

##
## Create one more file
##
writeLines("Hello world!", file.path(path, "test-2.txt"))

##
## Check that there are no stashes
##
stopifnot(identical(stashes(repo), list()))

##
## Stash
##
stash(repo)
stopifnot(identical(stashes(repo), list()))
stash(repo, untracked=TRUE)
stopifnot(identical(length(stashes(repo)), 1L))

##
## Drop stash
##
drop_stash(repo, 0)
stopifnot(identical(stashes(repo), list()))

##
## Check drop_stash argument
##
tools::assertError(drop_stash(repo))
tools::assertError(drop_stash(repo, -1))
tools::assertError(drop_stash(repo, 0.5))

##
## Cleanup
##
unlink(path, recursive=TRUE)
