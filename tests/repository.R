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
source("util/check.R")

## For debugging
sessionInfo()
libgit2_version()
libgit2_features()


## Create a directory in tempdir
path <- tempfile(pattern = "git2r-")
dir.create(path)

## is_bare: "Invalid repository"
tools::assertError(is_bare(new("git_repository")))

## is_empty: "Invalid repository"
tools::assertError(is_empty(new("git_repository")))

## Check that open an invalid repository fails
tools::assertError(repository(path))
tools::assertError(repository(path, discover = FALSE))

## Check that it fails to open/init a repository with a path to a
## file.
writeLines("test", file.path(path, "test.txt"))
tools::assertError(repository(file.path(path, "test.txt"),
                              discover = FALSE))
tools::assertError(init(file.path(path, "test.txt")))
unlink(file.path(path, "test.txt"))

## Initialize a repository
repo <- init(path)
stopifnot(identical(print(repo), repo))

## Check the state of the repository
stopifnot(identical(is_bare(repo), FALSE))
stopifnot(identical(is_empty(repo), TRUE))
stopifnot(identical(is_shallow(repo), FALSE))
stopifnot(identical(branches(repo), empty_named_list()))
stopifnot(identical(references(repo), empty_named_list()))
stopifnot(identical(commits(repo), list()))
stopifnot(identical(repository_head(repo), NULL))

# check that we can find repository from a path
wd <- sub(paste0("[", .Platform$file.sep, "]$"), "",  workdir(repo))
writeLines("test file", con = file.path(wd, "myfile.txt"))
stopifnot(identical(discover_repository(file.path(wd, "myfile.txt")),
                    file.path(wd, ".git")))
stopifnot(identical(discover_repository(file.path(wd, "doesntexist.txt")),
                    NULL))

# Check that we can use ceiling in discover repostiory
dir.create(file.path(wd, "temp"))
stopifnot(identical(discover_repository(file.path(wd, "temp"), 0), NULL))
stopifnot(identical(discover_repository(file.path(wd, "temp"), 1),
                    file.path(wd, ".git")))
tools::assertError(discover_repository(file.path(wd, "temp"), 2))

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
unlink(path, recursive = TRUE)
