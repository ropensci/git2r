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

## Initialize a repository
repo <- init(path)
config(repo, user.name = "Alice", user.email = "alice@example.org")

## Add files
invisible(lapply(file.path(path, letters[1:4]), writeLines, text = ""))
add(repo, letters)
commit(repo, "init")

## Remove one file
rm_file(repo, letters[1])
commit(repo, "remove")

## Remove two files. Don't raise warnings
withCallingHandlers(rm_file(repo, letters[2:3]), warning = function(w) stop(w))

## Remove one file using the absolute path to the file.
rm_file(repo, file.path(path, letters[4]))

## Cleanup
unlink(path, recursive = TRUE)
