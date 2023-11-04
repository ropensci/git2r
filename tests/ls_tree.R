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


## Initialize a temporary repository
path <- tempfile(pattern = "git2r-")
dir.create(path)
dir.create(file.path(path, "subfolder"))
repo <- init(path)

## Create a user
config(repo, user.name = "Alice", user.email = "alice@example.org")

## Create three files and commit
writeLines("First file",  file.path(path, "example-1.txt"))
writeLines("Second file", file.path(path, "subfolder/example-2.txt"))
writeLines("Third file",  file.path(path, "example-3.txt"))
add(repo, c("example-1.txt", "subfolder/example-2.txt", "example-3.txt"))
commit(repo, "Commit message")

## Traverse tree entries and its subtrees.
## Various approaches that give identical result.
stopifnot(identical(ls_tree(tree = tree(last_commit(path))),
                    ls_tree(tree = tree(last_commit(repo)))))
stopifnot(identical(ls_tree(repo = path), ls_tree(repo = repo)))

## ls_tree(repo = repo) should match `git ls-tree -lr HEAD`
ls_tree_result <- ls_tree(repo = repo)
stopifnot(identical(ls_tree_result$name,
                    c("example-1.txt", "example-3.txt", "example-2.txt")))

# Argument `tree` can be a  'character that identifies a tree in the repository'
ls_tree(tree = tree(last_commit(path))$sha, repo = repo)

## Skip content in subfolder
ls_tree_toplevel <- ls_tree(repo = repo, recursive = FALSE)
stopifnot(nrow(ls_tree_toplevel) == 3)
stopifnot(identical(ls_tree_toplevel$name,
                    c("example-1.txt", "example-3.txt", "subfolder")))

## Start in subfolder
ls_tree_subfolder <- ls_tree(tree = "HEAD:subfolder", repo = repo)
stopifnot(nrow(ls_tree_subfolder) == 1)
stopifnot(identical(ls_tree_subfolder$name, "example-2.txt"))

## Cleanup
unlink(path, recursive = TRUE)
