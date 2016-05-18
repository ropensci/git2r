## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2016 The git2r contributors
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

## Create a directory in tempdir
path <- tempfile(pattern="git2r-")
dir.create(path)

## Initialize a repository
repo <- init(path)
config(repo, user.name="Alice", user.email="alice@example.org")

## Commit without adding changes should produce an error
tools::assertError(commit(repo, "Test to commit"))

## Create a file
writeLines("Hello world!", file.path(path, "test.txt"))

## Commit without adding changes should produce an error
tools::assertError(commit(repo, "Test to commit"))

## Add and commit
add(repo, "test.txt")
commit_1 <- commit(repo, "Commit message")

## Check commit
stopifnot(identical(commit_1@author@name, "Alice"))
stopifnot(identical(commit_1@author@email, "alice@example.org"))
stopifnot(identical(lookup(repo, commit_1@sha), commit_1))
stopifnot(identical(length(commits(repo)), 1L))
stopifnot(identical(commits(repo)[[1]]@author@name, "Alice"))
stopifnot(identical(commits(repo)[[1]]@author@email, "alice@example.org"))
stopifnot(identical(parents(commit_1), list()))

## Check the commits method with other objects
tag_1 <- tag(repo, "Tagname", "Tag message")
stopifnot(identical(commits(tag_1), commits(repo)))
stopifnot(identical(commits(commit_1), commits(repo)))
stopifnot(identical(commits(branches(repo)$master), commits(repo)))

## Check is_commit
stopifnot(identical(is_commit(commit_1), TRUE))
stopifnot(identical(is_commit(5), FALSE))

## Commit without adding changes should produce an error
tools::assertError(commit(repo, "Test to commit"))

## Add another commit
writeLines(c("Hello world!", "HELLO WORLD!"), file.path(path, "test.txt"))
add(repo, "test.txt")
commit_2 <- commit(repo, "Commit message 2")

## Check relationship
stopifnot(identical(descendant_of(commit_2, commit_1), TRUE))
stopifnot(identical(descendant_of(commit_1, commit_2), FALSE))
stopifnot(identical(length(parents(commit_2)), 1L))
stopifnot(identical(parents(commit_2)[[1]], commit_1))

## Check contributions
stopifnot(identical(colnames(contributions(repo, by="author", breaks="day")),
                    c("when", "author", "n")))
stopifnot(identical(colnames(contributions(repo)),
                    c("when", "n")))
stopifnot(identical(nrow(contributions(repo)), 1L))
stopifnot(identical(contributions(repo)$n, 2L))
stopifnot(identical(contributions(repo, by="author", breaks="day")$n, 2L))

## Add another commit with 'all' argument
writeLines(c("Hello world!", "HELLO WORLD!", "HeLlO wOrLd!"),
           file.path(path, "test.txt"))
commit(repo, "Commit message 3", all = TRUE)

stopifnot(identical(status(repo),
                    structure(list(
                        staged = structure(list(), .Names = character(0)),
                        unstaged = structure(list(), .Names = character(0)),
                        untracked = structure(list(), .Names = character(0))),
                              .Names = c("staged", "unstaged", "untracked"),
                              class = "git_status")))

## Delete file and commit with 'all' argument
file.remove(file.path(path, "test.txt"))
commit(repo, "Commit message 4", all = TRUE)

stopifnot(identical(status(repo),
                    structure(list(
                        staged = structure(list(), .Names = character(0)),
                        unstaged = structure(list(), .Names = character(0)),
                        untracked = structure(list(), .Names = character(0))),
                              .Names = c("staged", "unstaged", "untracked"),
                              class = "git_status")))

## Check max number of commits in output
stopifnot(identical(length(commits(repo)), 4L))
stopifnot(identical(length(commits(repo, n = -1)), 4L))
stopifnot(identical(length(commits(repo, n = 2)), 2L))
tools::assertError(commits(repo, n = 2.2))
tools::assertError(commits(repo, n = "2"))

## Check to coerce repository to data.frame
df <- as(repo, "data.frame")
stopifnot(identical(dim(df), c(4L, 6L)))
stopifnot(identical(names(df), c("sha", "summary", "message",
                                 "author", "email", "when")))

## Set working directory to path and check commits
setwd(path)
stopifnot(identical(length(commits()), 4L))
stopifnot(identical(length(commits(n = -1)), 4L))
stopifnot(identical(length(commits(n = 2)), 2L))
tools::assertError(commits(n = 2.2))
tools::assertError(commits(n = "2"))

## Check plot method
plot_file <- tempfile(fileext = ".pdf")
pdf(plot_file)
plot(repo)
dev.off()
stopifnot(file.exists(plot_file))
unlink(plot_file)

## Check punch card plot method
punch_card_plot_file <- tempfile(fileext = ".pdf")
pdf(punch_card_plot_file)
punch_card(repo)
dev.off()
stopifnot(file.exists(punch_card_plot_file))
unlink(punch_card_plot_file)

## Check that 'git2r_arg_check_commit' raise error
res <- tools::assertError(.Call(git2r:::git2r_commit_tree, NULL))
stopifnot(length(grep("'commit' must be a S4 class git_commit",
                      res[[1]]$message)) > 0)
res <- tools::assertError(.Call(git2r:::git2r_commit_tree, 3))
stopifnot(length(grep("'commit' must be a S4 class git_commit",
                      res[[1]]$message)) > 0)
res <- tools::assertError(.Call(git2r:::git2r_commit_tree, repo))
stopifnot(length(grep("'commit' must be a S4 class git_commit",
                      res[[1]]$message)) > 0)
commit_1@sha <- NA_character_
res <- tools::assertError(.Call(git2r:::git2r_commit_tree, commit_1))
stopifnot(length(grep("'commit' must be a S4 class git_commit",
                      res[[1]]$message)) > 0)

## Cleanup
unlink(path, recursive=TRUE)
