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

library(git2r)
source("util/check.R")

## For debugging
sessionInfo()
libgit2_version()
libgit2_features()


## Create a directory in tempdir
path <- tempfile(pattern = "git2r-")
dir.create(path)

## Initialize a repository
repo <- init(path, branch = "main")
config(repo, user.name = "Alice", user.email = "alice@example.org")

## Create a file, add and commit
writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
           con = file.path(path, "test.txt"))
add(repo, "test.txt")
commit_1 <- commit(repo, "Commit message 1")

## Create first branch, checkout, add file and commit
b_1 <- branch_create(commit_1, "branch1")
checkout(b_1)
writeLines("Branch 1", file.path(path, "branch-1.txt"))
add(repo, "branch-1.txt")
commit_2 <- commit(repo, "Commit message branch 1")

## Create second branch, checkout, add file and commit
b_2 <- branch_create(commit_1, "branch2")
checkout(b_2)
writeLines("Branch 2", file.path(path, "branch-2.txt"))
add(repo, "branch-2.txt")
commit_3 <- commit(repo, "Commit message branch 2")
writeLines(c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
             "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
           con = file.path(path, "test.txt"))
add(repo, "test.txt")
commit_4 <- commit(repo, "Second commit message branch 2")

## Check that merge base equals commit_1
stopifnot(identical(merge_base(commit_2, commit_3), commit_1))

## Checkout main
b <- branches(repo)
checkout(b[sapply(b, "[", "name") == "main"][[1]], force = TRUE)

## Merge branch 1
m_1 <- merge(b[sapply(b, "[", "name") == "branch1"][[1]])
stopifnot(identical(m_1$fast_forward, TRUE))
stopifnot(identical(m_1$conflicts, FALSE))
stopifnot(identical(sha(m_1), NA_character_))
stopifnot(identical(print(m_1), m_1))

## Merge branch 1 again
m_1_again <- merge(b[sapply(b, "[", "name") == "branch1"][[1]])
stopifnot(identical(m_1_again$up_to_date, TRUE))
stopifnot(identical(m_1_again$fast_forward, FALSE))
stopifnot(identical(m_1_again$conflicts, FALSE))
stopifnot(identical(sha(m_1_again), NA_character_))

## Merge branch 2
m_2 <- merge(b[sapply(b, "[", "name") == "branch2"][[1]])
stopifnot(identical(m_2$fast_forward, FALSE))
stopifnot(identical(m_2$conflicts, FALSE))
stopifnot(identical(sha(m_2), sha(commits(repo)[[1]])))

## Create third branch, checkout, change file and commit
b_3 <- branch_create(lookup(repo, sha(m_2)), "branch3")
checkout(b_3)
writeLines(c("Lorem ipsum dolor amet sit, consectetur adipisicing elit, sed do",
             "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
           con = file.path(path, "test.txt"))
add(repo, "test.txt")
commit(repo, "Commit message branch 3")

## Checkout main and create a change that creates a conflict on
## merge
b <- branches(repo)
checkout(b[sapply(b, "[", "name") == "main"][[1]], force = TRUE)
writeLines(c("Lorem ipsum dolor sit amet, adipisicing consectetur elit, sed do",
             "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
           con = file.path(path, "test.txt"))
add(repo, "test.txt")
commit(repo, "Some commit message branch 1")

## Merge branch 3 with fail = TRUE
m_3 <- merge(b[sapply(b, "[", "name") == "branch3"][[1]], fail = TRUE)
stopifnot(identical(m_3$up_to_date, FALSE))
stopifnot(identical(m_3$fast_forward, FALSE))
stopifnot(identical(m_3$conflicts, TRUE))
stopifnot(identical(sha(m_3), NA_character_))
m_3

## Check status; Expect to have a clean working directory
wd <- structure(list(staged = empty_named_list(),
                     unstaged = empty_named_list(),
                     untracked = empty_named_list()),
                class = "git_status")
stopifnot(identical(status(repo), wd))

## Merge branch 3
m_3 <- merge(b[sapply(b, "[", "name") == "branch3"][[1]])
stopifnot(identical(m_3$up_to_date, FALSE))
stopifnot(identical(m_3$fast_forward, FALSE))
stopifnot(identical(m_3$conflicts, TRUE))
stopifnot(identical(sha(m_3), NA_character_))
m_3

## Check status; Expect to have one unstaged unmerged conflict.
stopifnot(identical(status(repo),
                    structure(list(staged = empty_named_list(),
                                   unstaged = list(conflicted = "test.txt"),
                                   untracked = empty_named_list()),
                              class = "git_status")))

## Cleanup
unlink(path, recursive = TRUE)
