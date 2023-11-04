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

## Create a file
writeLines("Hello world!", file.path(path, "test.txt"))

## Add and commit
add(repo, "test.txt")
commit_1 <- commit(repo, "Commit message")

## Add a remote
remote_add(repo, "playground",
           "https://github.com/gaborcsardi/playground")

stopifnot(identical(remotes(repo), "playground"))
stopifnot(identical(remote_url(repo, "playground"),
                    "https://github.com/gaborcsardi/playground"))
stopifnot(identical(remote_url(repo),
                    "https://github.com/gaborcsardi/playground"))

## Rename a remote
remote_rename(repo, "playground", "foobar")

stopifnot(identical(remotes(repo), "foobar"))
stopifnot(identical(remote_url(repo, "foobar"),
                    "https://github.com/gaborcsardi/playground"))

## Set remote url
remote_set_url(repo, "foobar", "https://github.com/stewid/playground")
stopifnot(identical(remote_url(repo, "foobar"),
                    "https://github.com/stewid/playground"))

## Remove a remote
remote_remove(repo, "foobar")

stopifnot(identical(remotes(repo), character(0)))

if (identical(Sys.getenv("NOT_CRAN"), "true")) {
    if (isTRUE(libgit2_features()$https)) {
        refs <- remote_ls("https://github.com/ropensci/git2r")
        stopifnot(length(refs) > 0)
        stopifnot(names(refs) > 0)
        stopifnot(any(names(refs) == "HEAD"))
    }
}

# an invalid URL should throw an error
tools::assertError(remote_ls("bad"))

## Cleanup
unlink(path, recursive = TRUE)
