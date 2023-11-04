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

library("git2r")

## For debugging
sessionInfo()
libgit2_version()
libgit2_features()


## Create a directory in tempdir
path <- tempfile(pattern = "git2r-")
dir.create(file.path(path, "bundle", "R"), recursive = TRUE)

## Initialize a repository
repo <- init(file.path(path, "bundle"))
config(repo, user.name = "Alice", user.email = "alice@example.org")

## Create a DESCRIPTION file
writeLines(c(
    "package: bundle",
    "Title: Bundle Git Repository",
    "Description: Bundle a bare repository of the code in the 'inst' folder.",
    "Version: 0.1",
    "License: GPL-2",
    "Authors@R: person('Alice', role = c('aut', 'cre'),",
    "                  email = 'alice@example.org')"),
    con = file.path(path, "bundle", "DESCRIPTION"))
add(repo, file.path(path, "bundle", "DESCRIPTION"))
commit(repo, "Add DESCRIPTION file")

## Create R file
writeLines("f <- function(x, y) x+y",
           con = file.path(path, "bundle", "R", "bundle.R"))
add(repo, file.path(path, "bundle", "R", "bundle.R"))
commit(repo, "Add R file")

## Bundle package
bundle_r_package(repo)

## Fails if bundled package exists
tools::assertError(bundle_r_package(repo))

## Cleanup
unlink(path, recursive = TRUE)
