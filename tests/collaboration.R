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
## :TODO:FIX: functionality to:
##  1) push
##  2) pull
##

##
## Test collaboration between a bare repository and two clones
##

##
## Create 3 directories in tempdir
##
path_bare <- tempfile(pattern="git2r-")
path_repo1 <- tempfile(pattern="git2r-")
path_repo2 <- tempfile(pattern="git2r-")

dir.create(path_bare)
dir.create(path_repo1)
dir.create(path_repo2)

##
## Create repositories
##
bare_repo <- init(path_bare, bare = TRUE)
repo1 <- clone(path_bare, path_repo1)
repo2 <- clone(path_bare, path_repo2)

##
## Check the repositores
##
stopifnot(identical(is.bare(bare_repo), TRUE))
stopifnot(identical(is.bare(repo1), FALSE))
stopifnot(identical(is.bare(repo2), FALSE))

##
## Config repositories
##
config(repo1, user.name="Repo One", user.email="repo.one@git2r.org")
config(repo2, user.name="Repo Two", user.email="repo.two@git2r.org")

##
## Add changes to repo1
##
writeLines("Hello world", con = file.path(path_repo1, "test.txt"))
add(repo1, "test.txt")
new_commit <- commit(repo1, "Commit message")

##
## Check commit
##
stopifnot(identical(new_commit@author@name, "Repo One"))
stopifnot(identical(new_commit@author@email, "repo.one@git2r.org"))
stopifnot(identical(length(commits(repo1)), 1L))
stopifnot(identical(commits(repo1)[[1]]@author@name, "Repo One"))
stopifnot(identical(commits(repo1)[[1]]@author@email, "repo.one@git2r.org"))

##
## Collaborate
##
## push(repo1)

##
## Pull changes to repo2
##
## pull(repo2)
## stopifnot(identical(commits(repo1), commits(repo2)))

##
## Cleanup
##
unlink(path_bare, recursive=TRUE)
unlink(path_repo1, recursive=TRUE)
unlink(path_repo2, recursive=TRUE)
