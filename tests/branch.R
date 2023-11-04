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

## Create a file
writeLines("Hello world!", file.path(path, "test.txt"))

## add and commit
add(repo, "test.txt")
commit_1 <- commit(repo, "Commit message")

## Check branch
stopifnot(identical(length(branches(repo)), 1L))
stopifnot(identical(is_head(branches(repo)[[1]]), TRUE))
stopifnot(identical(is_local(branches(repo)[[1]]), TRUE))
stopifnot(identical(branches(repo)[[1]]$name, "main"))
stopifnot(identical(branches(repo)[[1]], repository_head(repo)))
stopifnot(identical(branches(repo)$main, repository_head(repo)))

## Check branch argument
res <- tools::assertError(is_local(1))
stopifnot(length(grep("argument 'branch' must be a 'git_branch' object",
                      res[[1]]$message)) > 0)

## Check branch name argument
res <- tools::assertError(branch_set_upstream())
stopifnot(length(grep("Missing argument name",
                      res[[1]]$message)) > 0)

## Print branch
b <- repository_head(repo)
stopifnot(identical(print(b), b))

## Create a branch
b <- branch_create(commit_1, name = "test")
stopifnot(identical(b$name, "test"))
stopifnot(identical(b$type, 1L))
stopifnot(identical(sha(b), branch_target(b)))
stopifnot(identical(length(branches(repo)), 2L))
stopifnot(identical(branch_target(branches(repo)[[1]]),
                    branch_target(branches(repo)[[2]])))

## Check is_branch
stopifnot(identical(is_branch(b), TRUE))
stopifnot(identical(is_branch(5), FALSE))

## Add one more commit
writeLines(c("Hello world!", "HELLO WORLD!"), file.path(path, "test.txt"))
add(repo, "test.txt")
commit_2 <- commit(repo, "Another commit message")

## Now the first branch should have moved on
stopifnot(!identical(branch_target(branches(repo)[[1]]),
                     branch_target(branches(repo)[[2]])))

## Create a branch with the same name should fail
tools::assertError(branch_create(commit_2, name = "test"))

## Force it and check the branches are identical again
b <- branch_create(commit_2, name = "test", force = TRUE)
stopifnot(identical(branch_target(branches(repo)[[1]]),
                    branch_target(branches(repo)[[2]])))

## Test arguments
res <- tools::assertError(.Call(git2r:::git2r_branch_delete, NULL))
stopifnot(length(grep("'branch' must be an S3 class git_branch",
                      res[[1]]$message)) > 0)
res <- tools::assertError(.Call(git2r:::git2r_branch_delete, 3))
stopifnot(length(grep("'branch' must be an S3 class git_branch",
                      res[[1]]$message)) > 0)
res <- tools::assertError(.Call(git2r:::git2r_branch_delete, repo))
stopifnot(length(grep("'branch' must be an S3 class git_branch",
                      res[[1]]$message)) > 0)
b_tmp <- b
b_tmp$name <- NA_character_
res <- tools::assertError(.Call(git2r:::git2r_branch_delete, b_tmp))
stopifnot(length(grep("'branch' must be an S3 class git_branch",
                      res[[1]]$message)) > 0)
b_tmp <- b
b_tmp$type <- NA_integer_
res <- tools::assertError(.Call(git2r:::git2r_branch_delete, b_tmp))
stopifnot(length(grep("'branch' must be an S3 class git_branch",
                      res[[1]]$message)) > 0)
b_tmp$type <- 3L
res <- tools::assertError(.Call(git2r:::git2r_branch_delete, b_tmp))
stopifnot(length(grep("'branch' must be an S3 class git_branch",
                      res[[1]]$message)) > 0)

## Delete branch
branch_delete(b)
stopifnot(identical(length(branches(repo)), 1L))

## Add one more commit
writeLines(c("Hello world!", "HELLO WORLD!", "hello world"),
           file.path(path, "test.txt"))
add(repo, "test.txt")
commit_3 <- commit(repo, "Another third commit message")

## Create and test renaming of branches
b_1 <- branch_create(commit_1, name = "test-1")
b_2 <- branch_create(commit_2, name = "test-2")
b_3 <- branch_create(commit_3, name = "test-3")
stopifnot(identical(length(branches(repo)), 4L))
b_1 <- branch_rename(b_1, name = "test-1-new-name")
stopifnot(identical(length(branches(repo)), 4L))
stopifnot(identical(b_1$name, "test-1-new-name"))
tools::assertError(branch_rename(b_1, name = "test-2"))
branch_rename(b_1, name = "test-2", force = TRUE)
stopifnot(identical(length(branches(repo)), 3L))

## Check branches method with missing repo argument
wd <- setwd(path)
stopifnot(identical(length(branches()), 3L))
if (!is.null(wd))
    setwd(wd)

## Cleanup
unlink(path, recursive = TRUE)
