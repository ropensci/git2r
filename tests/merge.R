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

## Create a directory in tempdir
path <- tempfile(pattern="git2r-")
dir.create(path)

## Initialize a repository
repo <- init(path)
config(repo, user.name="Developer", user.email="developer@example.org")

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

## Checkout master
b <- branches(repo)
checkout(b[sapply(b, slot, "name") == "master"][[1]], force=TRUE)

## Merge branch 1
m_1 <- git2r:::merge_branch(
    b[sapply(b, slot, "name") == "branch1"][[1]],
    commit_on_success = TRUE,
    merger = default_signature(repo))

stopifnot(identical(m_1@fast_forward, TRUE))
stopifnot(identical(m_1@conflicts, FALSE))
stopifnot(identical(m_1@sha, character(0)))

## Merge branch 2
m_2 <- git2r:::merge_branch(
    b[sapply(b, slot, "name") == "branch2"][[1]],
    commit_on_success = TRUE,
    merger = default_signature(repo))

stopifnot(identical(m_2@fast_forward, FALSE))
stopifnot(identical(m_2@conflicts, FALSE))
stopifnot(identical(m_2@sha, commits(repo)[[1]]@sha))

## Create third branch, checkout, change file and commit
b_3 <- branch_create(lookup(repo, m_2@sha), "branch3")
checkout(b_3)
writeLines(c("Lorem ipsum dolor amet sit, consectetur adipisicing elit, sed do",
             "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
           con = file.path(path, "test.txt"))
add(repo, "test.txt")
commit(repo, "Commit message branch 3")

## Checkout master and create a change that creates a conflict on
## merge
b <- branches(repo)
checkout(b[sapply(b, slot, "name") == "master"][[1]], force=TRUE)
writeLines(c("Lorem ipsum dolor sit amet, adipisicing consectetur elit, sed do",
             "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
           con = file.path(path, "test.txt"))
add(repo, "test.txt")
commit(repo, "Some commit message branch 1")

## Merge branch 3
m_3 <- git2r:::merge_branch(
    b[sapply(b, slot, "name") == "branch3"][[1]],
    commit_on_success = TRUE,
    merger = default_signature(repo))

stopifnot(identical(m_3@up_to_date, FALSE))
stopifnot(identical(m_3@fast_forward, FALSE))
stopifnot(identical(m_3@conflicts, TRUE))
stopifnot(identical(m_3@sha, character(0)))

## Check status; Expect to have one unstaged unmerged conflict.
stopifnot(identical(status(repo, verbose = FALSE),
                    structure(list(staged = structure(list(),
                                       .Names = character(0)),
                                   unstaged = structure(list(unmerged = "test.txt"),
                                       .Names = "unmerged"),
                                   untracked = structure(list(),
                                       .Names = character(0))),
                              .Names = c("staged", "unstaged", "untracked"))))

## Cleanup
unlink(path, recursive=TRUE)
