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
## :TODO:FIX: functionality to:
##  1) configure user.name and user.email
##  1) push
##  2) pull
##

##
## Test collaboration between a bare repository and two clones
##

##
## Create 3 directories in tempdir
##
path_bare <- tempfile(pattern='git2r-')
path_repo1 <- tempfile(pattern='git2r-')
path_repo2 <- tempfile(pattern='git2r-')

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
## Add changes to repo1
##
## writeLines('Hello world', con = file.path(path_repo1, 'test.txt'))
## add(repo1, 'test.txt')
## commit(repo1, 'Commit message')
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
