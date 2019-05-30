## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2019 The git2r contributors
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

## Create two files and alternate commits
writeLines("1", file.path(path, "odd.txt"))
add(repo, "odd.txt")
c1 <- commit(repo, "commit 1")

writeLines("2", file.path(path, "even.txt"))
add(repo, "even.txt")
c2 <- commit(repo, "commit 2")

writeLines("3", file.path(path, "odd.txt"))
add(repo, "odd.txt")
c3 <- commit(repo, "commit 3")

writeLines("4", file.path(path, "even.txt"))
add(repo, "even.txt")
c4 <- commit(repo, "commit 4")

writeLines("5", file.path(path, "odd.txt"))
add(repo, "odd.txt")
c5 <- commit(repo, "commit 5")

writeLines("6", file.path(path, "even.txt"))
add(repo, "even.txt")
c6 <- commit(repo, "commit 6")

commits_all <- commits(repo)
stopifnot(length(commits_all) == 6)

## Test path
commits_odd <- commits(repo, path = "odd.txt")
stopifnot(length(commits_odd) == 3)
stopifnot(commits_odd[[1]]$sha == c5$sha)
stopifnot(commits_odd[[2]]$sha == c3$sha)
stopifnot(commits_odd[[3]]$sha == c1$sha)

commits_even <- commits(repo, path = "even.txt")
stopifnot(length(commits_even) == 3)
stopifnot(commits_even[[1]]$sha == c6$sha)
stopifnot(commits_even[[2]]$sha == c4$sha)
stopifnot(commits_even[[3]]$sha == c2$sha)

## Test reverse
commits_odd_rev <- commits(repo, reverse = TRUE, path = "odd.txt")
stopifnot(length(commits_odd_rev) == 3)
stopifnot(commits_odd_rev[[1]]$sha == c1$sha)
stopifnot(commits_odd_rev[[2]]$sha == c3$sha)
stopifnot(commits_odd_rev[[3]]$sha == c5$sha)

commits_even_rev <- commits(repo, reverse = TRUE, path = "even.txt")
stopifnot(length(commits_even_rev) == 3)
stopifnot(commits_even_rev[[1]]$sha == c2$sha)
stopifnot(commits_even_rev[[2]]$sha == c4$sha)
stopifnot(commits_even_rev[[3]]$sha == c6$sha)

## Test n
commits_odd_n <- commits(repo, n = 2, path = "odd.txt")
stopifnot(length(commits_odd_n) == 2)
stopifnot(commits_odd_n[[1]]$sha == c5$sha)
stopifnot(commits_odd_n[[2]]$sha == c3$sha)

commits_even_n <- commits(repo, n = 2, path = "even.txt")
stopifnot(length(commits_even_n) == 2)
stopifnot(commits_even_n[[1]]$sha == c6$sha)
stopifnot(commits_even_n[[2]]$sha == c4$sha)

commits_odd_0 <- commits(repo, n = 0, path = "odd.txt")
stopifnot(length(commits_odd_0) == 0)
stopifnot(identical(commits_odd_0, list()))

commits_even_0 <- commits(repo, n = 0, path = "even.txt")
stopifnot(length(commits_even_0) == 0)
stopifnot(identical(commits_even_0, list()))

## Test ref
checkout(repo, branch = "test-ref", create = TRUE)

writeLines("7", file.path(path, "odd.txt"))
add(repo, "odd.txt")
c7 <- commit(repo, "commit 7")

writeLines("8", file.path(path, "even.txt"))
add(repo, "even.txt")
c8 <- commit(repo, "commit 8")

commits_odd_ref <- commits(repo, ref = "master", path = "odd.txt")
stopifnot(length(commits_odd_ref) == 3)
stopifnot(commits_odd_ref[[1]]$sha == c5$sha)
stopifnot(commits_odd_ref[[2]]$sha == c3$sha)
stopifnot(commits_odd_ref[[3]]$sha == c1$sha)

commits_even_ref <- commits(repo, ref = "master", path = "even.txt")
stopifnot(length(commits_even_ref) == 3)
stopifnot(commits_even_ref[[1]]$sha == c6$sha)
stopifnot(commits_even_ref[[2]]$sha == c4$sha)
stopifnot(commits_even_ref[[3]]$sha == c2$sha)

checkout(repo, branch = "master")

## Test renaming a file (path does not support --follow)
writeLines("a file to be renamed", file.path(path, "original.txt"))
add(repo, "original.txt")
c_original <- commit(repo, "commit original")

commits_original <- commits(repo, path = "original.txt")
stopifnot(length(commits_original) == 1)
stopifnot(commits_original[[1]]$sha == c_original$sha)

file.rename(file.path(path, "original.txt"), file.path(path, "new.txt"))
add(repo, c("original.txt", "new.txt"))
c_new <- commit(repo, "commit new")

commits_new <- commits(repo, path = "new.txt")
stopifnot(length(commits_new) == 1)
stopifnot(commits_new[[1]]$sha == c_new$sha)

## Test merge commits
writeLines(letters[1:5], file.path(path, "merge.txt"))
add(repo, "merge.txt")
c_merge_1 <- commit(repo, "commit merge 1")

checkout(repo, branch = "test-merge", create = TRUE)
cat("z", file = file.path(path, "merge.txt"), append = TRUE)
add(repo, "merge.txt")
c_merge_2 <- commit(repo, "commit merge 2")

checkout(repo, branch = "master")
writeLines(c("A", letters[2:5]), file.path(path, "merge.txt"))
add(repo, "merge.txt")
c_merge_3 <- commit(repo, "commit merge 3")

c_merge_4 <- merge(repo, "test-merge")
stopifnot(class(c_merge_4) == "git_merge_result")

commits_merge <- commits(repo, path = "merge.txt")
stopifnot(length(commits_merge) == 4)
stopifnot(commits_merge[[1]]$sha == c_merge_4$sha)
stopifnot(commits_merge[[2]]$sha == c_merge_3$sha)
stopifnot(commits_merge[[3]]$sha == c_merge_2$sha)
stopifnot(commits_merge[[4]]$sha == c_merge_1$sha)

## Test absolute path
writeLines("absolute", file.path(path, "abs.txt"))
add(repo, "abs.txt")
c_abs <- commit(repo, "commit absolute")

commits_abs <- commits(repo, path = file.path(path, "abs.txt"))
stopifnot(length(commits_abs) == 1)
stopifnot(commits_abs[[1]]$sha == c_abs$sha)

## Cleanup
unlink(path, recursive=TRUE)
