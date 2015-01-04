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

##
## Create a directory in tempdir
##
path <- tempfile(pattern="git2r-")
dir.create(path)

##
## Initialize a repository
##
repo <- init(path)
config(repo, user.name="Repo", user.email="repo@example.org")

##
## Create a file
##
writeLines("Hello world!", file.path(path, "test.txt"))

##
## Add and commit
##
add(repo, "test.txt")
commit_1 <- commit(repo, "Commit message")

##
## Add a remote
##
remote_add(repo, "playground",
           "https://github.com/gaborcsardi/playground")

stopifnot(identical(remotes(repo), "playground"))
stopifnot(identical(remote_url(repo, "playground"),
                    "https://github.com/gaborcsardi/playground"))

##
## Rename a remote
##
remote_rename(repo, "playground", "foobar")

stopifnot(identical(remotes(repo), "foobar"))
stopifnot(identical(remote_url(repo, "foobar"),
                    "https://github.com/gaborcsardi/playground"))

##
## Remove a remote
##
remote_remove(repo, "foobar")

stopifnot(identical(remotes(repo), character(0)))

##
## Cleanup
##
unlink(path, recursive=TRUE)
