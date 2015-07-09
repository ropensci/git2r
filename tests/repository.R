## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2015 The git2r contributors
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

## Create a directory in tempdir
path <- tempfile(pattern="git2r-")
dir.create(path)

## is_bare: "Invalid repository"
tools::assertError(is_bare(new("git_repository")))

## is_empty: "Invalid repository"
tools::assertError(is_empty(new("git_repository")))

## Check that open an invalid repository fails
tools::assertError(repository(path))

## Initialize a repository
repo <- init(path)

## Check the state of the repository
stopifnot(validObject(repo))
stopifnot(identical(is_bare(repo), FALSE))
stopifnot(identical(is_empty(repo), TRUE))
stopifnot(identical(is_shallow(repo), FALSE))
stopifnot(identical(branches(repo), structure(list(), .Names = character(0))))
stopifnot(identical(references(repo), structure(list(), .Names = character(0))))
stopifnot(identical(commits(repo), list()))
stopifnot(identical(head(repo), NULL))

# check that we can find repository from a path
wd <- workdir(repo)
writeLines('test file', con=file.path(wd, 'myfile.txt'))
stopifnot(identical(discover_repository(file.path(wd, 'myfile.txt')),
                    paste0(wd, '.git', .Platform$file.sep)))
stopifnot(identical(discover_repository(file.path(wd, 'doesntexist.txt')),
                    NULL))

## Check that lookup with a sha of less than 4 characters or more than
## 40 characters fail.
tools::assertError(lookup(repo, paste0(rep("a", 3), collapse = "")))
tools::assertError(lookup(repo, paste0(rep("a", 41), collapse = "")))

## Check in_repository
stopifnot(identical(in_repository(path), TRUE))

## Check:
## - in_repository method with missing path argument
## - repository method with missing path argument
## - workdir method with missing path argument
## - is_empty method with missing repo argument
## - is_shallow method with missing repo argument
wd <- setwd(path)
stopifnot(identical(in_repository(), TRUE))
stopifnot(identical(workdir(repository(path)), workdir(repository())))
stopifnot(identical(workdir(repository(path)), workdir()))
stopifnot(identical(is_empty(), TRUE))
stopifnot(identical(is_shallow(), FALSE))
if (!is.null(wd))
    setwd(wd)

## Cleanup
unlink(path, recursive=TRUE)
