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

## Create a file, add, commit
writeLines("Hello world!", file.path(path, "test.txt"))
add(repo, "test.txt")
commit(repo, "Commit message")

## Change the file, diff between index and workdir
writeLines("Hello again!\nHere is a second line\nAnd a third",
           file.path(path, "test.txt"))
diff_1 <- diff(repo)
diff(repo, as_char = TRUE)
diff(repo, as_char = TRUE, filename = file.path(path, "test.diff"))

stopifnot(identical(diff_1$old, "index"))
stopifnot(identical(diff_1$new, "workdir"))
stopifnot(identical(length(diff_1$files), 1L))
stopifnot(identical(diff_1$files[[1]]$old_file, "test.txt"))
stopifnot(identical(diff_1$files[[1]]$new_file, "test.txt"))
stopifnot(identical(length(diff_1$files[[1]]$hunks), 1L))
stopifnot(identical(length(diff_1$files[[1]]$hunks[[1]]$lines), 4L))
## TODO: check actual diff

## Diff between index and HEAD is empty
diff_2 <- diff(repo, index = TRUE)
diff(repo, as_char = TRUE)
diff(repo, as_char = TRUE, filename = file.path(path, "test.diff"))

stopifnot(identical(diff_2$old, "HEAD"))
stopifnot(identical(diff_2$new, "index"))
stopifnot(identical(diff_2$files, list()))

## Diff between tree and working dir, same as diff_1
diff_3 <- diff(tree(commits(repo)[[1]]))
diff(repo, as_char = TRUE)
diff(repo, as_char = TRUE, filename = file.path(path, "test.diff"))

stopifnot(identical(diff_3$old, tree(commits(repo)[[1]])))
stopifnot(identical(diff_3$new, "workdir"))
stopifnot(identical(diff_3$files, diff_1$files))
stopifnot(identical(print(diff_3), diff_3))

## Add changes, diff between index and HEAD is the same as diff_1
add(repo, "test.txt")
diff_4 <- diff(repo, index = TRUE)
diff(repo, as_char = TRUE)
diff(repo, as_char = TRUE, filename = file.path(path, "test.diff"))

stopifnot(identical(diff_4$old, "HEAD"))
stopifnot(identical(diff_4$new, "index"))
stopifnot(identical(diff_4$files, diff_1$files))

## Diff between tree and index
diff_5 <- diff(tree(commits(repo)[[1]]), index = TRUE)
diff(repo, as_char = TRUE)
diff(repo, as_char = TRUE, filename = file.path(path, "test.diff"))

stopifnot(identical(diff_5$old, tree(commits(repo)[[1]])))
stopifnot(identical(diff_5$new, "index"))
stopifnot(identical(diff_5$files, diff_1$files))

## Diff between two trees
commit(repo, "Second commit")
tree_1 <- tree(commits(repo)[[2]])
tree_2 <- tree(commits(repo)[[1]])
diff_6 <- diff(tree_1, tree_2)
diff(repo, as_char = TRUE)
diff(repo, as_char = TRUE, filename = file.path(path, "test.diff"))

stopifnot(identical(diff_6$old, tree_1))
stopifnot(identical(diff_6$new, tree_2))
stopifnot(identical(diff_6$files, diff_1$files))

## Length of a diff
stopifnot(identical(length(diff_1), 1L))
stopifnot(identical(length(diff_2), 0L))
stopifnot(identical(length(diff_3), 1L))
stopifnot(identical(length(diff_4), 1L))
stopifnot(identical(length(diff_5), 1L))
stopifnot(identical(length(diff_6), 1L))

## Binary files
set.seed(42)
writeBin(as.raw((sample(0:255, 1000, replace = TRUE))),
         con = file.path(path, "test.bin"))
add(repo, "test.bin")
diff_7 <- diff(repo, index = TRUE)
diff(repo, as_char = TRUE)
diff(repo, as_char = TRUE, filename = file.path(path, "test.diff"))

stopifnot(any(grepl("binary file", capture.output(summary(diff_7)))))

## TODO: errors
## Check non-logical index argument
res <- tools::assertError(
                  .Call(git2r:::git2r_diff, NULL, NULL, NULL, "FALSE",
                        NULL, 3L, 0L, "a", "b", NULL, NULL, NULL))
stopifnot(length(grep(paste0("Error in 'git2r_diff': 'index' must be logical ",
                             "vector of length one with non NA value\n"),
                      res[[1]]$message)) > 0)

## Check various combinations of diff arguments
res <- tools::assertError(
                  .Call(git2r:::git2r_diff, NULL, NULL,
                        tree(commits(repo)[[1]]),
                        FALSE, NULL, 3L, 0L, "a", "b", NULL, NULL, NULL))
stopifnot(length(grep("Error in 'git2r_diff': Invalid diff parameters",
                      res[[1]]$message)) > 0)

res <- tools::assertError(
                  .Call(git2r:::git2r_diff, NULL, NULL,
                        tree(commits(repo)[[1]]),
                        TRUE, NULL, 3L, 0L, "a", "b", NULL, NULL, NULL))
stopifnot(length(grep("Error in 'git2r_diff': Invalid diff parameters",
                      res[[1]]$message)) > 0)

res <- tools::assertError(
                  .Call(git2r:::git2r_diff, repo, tree(commits(repo)[[1]]),
                        NULL, FALSE, NULL, 3L, 0L, "a", "b", NULL, NULL, NULL))
stopifnot(length(grep("Error in 'git2r_diff': Invalid diff parameters",
                      res[[1]]$message)) > 0)

res <- tools::assertError(
                  .Call(git2r:::git2r_diff, repo, tree(commits(repo)[[1]]),
                        NULL, TRUE, NULL, 3L, 0L, "a", "b", NULL, NULL, NULL))
stopifnot(length(grep("Error in 'git2r_diff': Invalid diff parameters",
                      res[[1]]$message)) > 0)

res <- tools::assertError(
                  .Call(git2r:::git2r_diff, repo, tree(commits(repo)[[1]]),
                        tree(commits(repo)[[2]]), FALSE, NULL, 3L, 0L, "a",
                        "b", NULL, NULL, NULL))
stopifnot(length(grep("Error in 'git2r_diff': Invalid diff parameters",
                      res[[1]]$message)) > 0)

res <- tools::assertError(
                  .Call(git2r:::git2r_diff, repo, tree(commits(repo)[[1]]),
                        tree(commits(repo)[[2]]), TRUE, NULL, 3L, 0L, "a",
                        "b", NULL, NULL, NULL))
stopifnot(length(grep("Error in 'git2r_diff': Invalid diff parameters",
                      res[[1]]$message)) > 0)

## TODO: printing

## Cleanup
unlink(path, recursive = TRUE)
