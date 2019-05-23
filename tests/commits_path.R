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

# Create two files and alternate commits
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

## Cleanup
unlink(path, recursive=TRUE)
