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
dir.create(path)

## Initialize a bare repository
repo <- init(path, bare = TRUE)

## Check that the state of the repository
stopifnot(identical(is_bare(repo), TRUE))
stopifnot(identical(is_empty(repo), TRUE))

## Check that workdir is NULL for a bare repository
stopifnot(is.null(workdir(repo)))

## Check with missing repo argument
setwd(path)
stopifnot(identical(is_bare(), TRUE))

## Cleanup
unlink(path, recursive = TRUE)
