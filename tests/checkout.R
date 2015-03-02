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
config(repo, user.name="Alice", user.email="alice@example.org")

##
## Create first commit
##
writeLines("Hello world!", file.path(path, "test.txt"))
add(repo, 'test.txt')
commit.1 <- commit(repo, "First commit message")

##
## Create second commit
##
writeLines(c("Hello world!", "HELLO WORLD!"), file.path(path, "test.txt"))
add(repo, 'test.txt')
commit.2 <- commit(repo, "Second commit message")
tag(repo, "commit.2", "Tag message")

##
## Create third commit
##
writeLines(c("Hello world!", "HELLO WORLD!", "HeLlO wOrLd!"),
           file.path(path, "test.txt"))
add(repo, 'test.txt')
commit.3 <- commit(repo, "Third commit message")

##
## Check HEAD
##
stopifnot(identical(is_detached(repo), FALSE))
stopifnot(identical(head(repo)@name, "master"))

##
## Checkout first commit
##
checkout(commit.1, TRUE)
stopifnot(identical(is_detached(repo), TRUE))
stopifnot(identical(head(repo), commit.1))
stopifnot(identical(readLines(file.path(path, "test.txt")), "Hello world!"))

##
## Checkout tag
##
checkout(tags(repo)[[1]], TRUE)
stopifnot(identical(is_detached(repo), TRUE))
stopifnot(identical(readLines(file.path(path, "test.txt")),
                    c("Hello world!", "HELLO WORLD!")))

##
## Cleanup
##
unlink(path, recursive=TRUE)
