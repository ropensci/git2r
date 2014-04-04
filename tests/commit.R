## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2014  Stefan Widgren
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, version 2 of the License.
##
## git2r is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
config(repo, user.name="Stefan Widgren", user.email="stefan.widgren@gmail.com")

##
## Create a file, add and commit
##
writeLines("Hello world!", file.path(path, "test.r"))
add(repo, 'test.r')
new_commit <- commit(repo, "Commit message")

##
## Check commit
##
stopifnot(identical(new_commit@author@name, "Stefan Widgren"))
stopifnot(identical(new_commit@author@email, "stefan.widgren@gmail.com"))
stopifnot(identical(length(commits(repo)), 1L))
stopifnot(identical(commits(repo)[[1]]@author@name, "Stefan Widgren"))
stopifnot(identical(commits(repo)[[1]]@author@email, "stefan.widgren@gmail.com"))

##
## Commit without adding changes should produce an error
##
tools::assertError(commit(repo, "Test to commit"))

##
## Cleanup
##
unlink(path, recursive=TRUE)
