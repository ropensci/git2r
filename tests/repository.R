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
## is_bare: "Invalid repository"
##
tools::assertError(is_bare(new("git_repository")))

##
## is_empty: "Invalid repository"
##
tools::assertError(is_empty(new("git_repository")))

##
## Check that open an invalid repository fails
##
tools::assertError(repository(path))

##
## Initialize a repository
##
repo <- init(path)

##
## Check that the state of the repository
##
stopifnot(identical(is_bare(repo), FALSE))
stopifnot(identical(is_empty(repo), TRUE))
stopifnot(identical(branches(repo), list()))
stopifnot(identical(commits(repo), list()))
stopifnot(identical(head(repo), NULL))

# check that we can find workdir for paths in repository
wd=workdir(repo)
stopifnot(identical(workdir(wd), wd))
writeLines('test file', con=file.path(wd, 'myfile.txt'))
stopifnot(identical(workdir(file.path(wd, 'myfile.txt')), wd))
stopifnot(identical(repository_discover(file.path(wd, 'myfile.txt')),
                    paste0(wd, '.git', .Platform$file.sep)))
stopifnot(identical(
  repository_discover(file.path(wd, 'doesntexist.txt')), NULL))

##
## Check that lookup with a hex of less than 4 characters or more than
## 40 characters fail.
##
tools::assertError(lookup(repo, paste0(rep("a", 3), collapse="")))
tools::assertError(lookup(repo, paste0(rep("a", 41), collapse="")))

##
## Cleanup
##
unlink(path, recursive=TRUE)
