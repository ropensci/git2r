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
f <- file(file.path(path, "test.txt"), "wb")
writeChar("Hello world!\n", f, eos = NULL)
close(f)

## add and commit
add(repo, "test.txt")
commit(repo, "Commit message")

## Check tree
stopifnot(is_tree(lookup(repo, "a0b0b9e615e9e433eb5f11859e9feac4564c58c5")))
stopifnot(identical(
    sha(lookup(repo, "a0b0b9e615e9e433eb5f11859e9feac4564c58c5")),
    "a0b0b9e615e9e433eb5f11859e9feac4564c58c5"))
stopifnot(is_tree(tree(commits(repo)[[1]])))
stopifnot(identical(lookup(repo, "a0b0b9e615e9e433eb5f11859e9feac4564c58c5"),
                    tree(commits(repo)[[1]])))
stopifnot(identical(length(tree(commits(repo)[[1]])), 1L))

## Coerce to a data.frame and check column names
stopifnot(identical(names(as.data.frame(tree(commits(repo)[[1]]))),
                    c("mode", "type", "sha", "name")))

## Coerce to list and check length
stopifnot(identical(length(as.list(tree(last_commit(repo)))), 1L))

## Print and summary
stopifnot(identical(print(tree(last_commit(repo))), tree(last_commit(repo))))
summary(tree(last_commit(repo)))

## Check indexing
stopifnot(is_blob(tree(last_commit(repo))[TRUE]))
stopifnot(is_blob(tree(last_commit(repo))["test.txt"]))
res <- tools::assertError(tree(last_commit(repo))[data.frame()])
stopifnot(length(grep("Invalid index", res[[1]]$message)) > 0)

## Check ls_tree
stopifnot(identical(ls_tree(repo = repo), ls_tree(repo = path)))
stopifnot(identical(ls_tree(tree = sha(tree(last_commit(repo))), repo = repo),
                    ls_tree(repo = repo)))

## Cleanup
unlink(path, recursive = TRUE)
