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
## Initialize a repository
##
repo <- init(path)
config(repo, user.name="Repo", user.email="repo@example.org")

##
## Create a file
##
writeLines("Hello world!", file.path(path, "test.txt"))

##
## add and commit
##
add(repo, "test.txt")
new_commit <- commit(repo, "Commit message")

##
## Lookup blob
##
blob <- lookup(repo, "cd0875583aabe89ee197ea133980a9085d08e497")
stopifnot(is(blob, "git_blob"))
stopifnot(identical(is.binary(blob), FALSE))
stopifnot(identical(blob, lookup(repo, "cd0875")))
stopifnot(identical(length(blob), 13L))
stopifnot(identical(content(blob), "Hello world!"))

##
## Add one more commit
## 
writeLines(c("Hello world!", "HELLO WORLD!", "HeLlO wOrLd!"),
           file.path(path, "test.txt"))
add(repo, "test.txt")
blob <- tree(commit(repo, "New commit message"))[1]
stopifnot(identical(content(blob),
                    c("Hello world!", "HELLO WORLD!", "HeLlO wOrLd!")))

##
## Cleanup
##
unlink(path, recursive=TRUE)
