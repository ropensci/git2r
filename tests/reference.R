## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2018 The git2r contributors
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

## Create a directory in tempdir
path <- tempfile(pattern="git2r-")
dir.create(path)

## Initialize a repository
repo <- init(path)
config(repo, user.name="Alice", user.email="alice@example.org")

## Create a file
writeLines("Hello world!", file.path(path, "test.txt"))

## add and commit
add(repo, "test.txt")
commit(repo, "Commit message")

## Check dwim of reference shorthand
stopifnot(identical(.Call(git2r:::git2r_reference_dwim, repo, "")$name,
                    "refs/heads/master"))
stopifnot(identical(.Call(git2r:::git2r_reference_dwim, repo, "master")$name,
                    "refs/heads/master"))
stopifnot(identical(.Call(git2r:::git2r_reference_dwim, repo, "refs/heads/master")$name,
                    "refs/heads/master"))

## print reference
r <- .Call(git2r:::git2r_reference_dwim, repo, "refs/heads/master")
stopifnot(identical(print(r), r))

## Cleanup
unlink(path, recursive=TRUE)
