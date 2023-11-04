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
repo <- init(path)
config(repo, user.name = "Alice", user.email = "alice@example.org")

## Commit without adding changes should produce an error
tools::assertError(commit(repo, "Test to commit"))

## Create a file
writeLines("Hello world!", file.path(path, "test.txt"))

## Commit without adding changes should produce an error
tools::assertError(commit(repo, "Test to commit"))

## Add
add(repo, "test.txt")

## Commit with empty message should produce an error
tools::assertError(commit(repo, ""))

## Commit
commit_1 <- commit(repo, "Commit message", session = TRUE)
summary(commit_1)
tag_1 <- tag(repo, "Tagname1", "Tag message 1")

## Check commit
stopifnot(identical(commit_1$author$name, "Alice"))
stopifnot(identical(commit_1$author$email, "alice@example.org"))
stopifnot(identical(lookup(repo, sha(commit_1)), commit_1))
stopifnot(identical(length(commits(repo)), 1L))
stopifnot(identical(commits(repo)[[1]]$author$name, "Alice"))
stopifnot(identical(commits(repo)[[1]]$author$email, "alice@example.org"))
stopifnot(identical(parents(commit_1), list()))
stopifnot(identical(print(commit_1), commit_1))

## Check is_commit
stopifnot(identical(is_commit(commit_1), TRUE))
stopifnot(identical(is_commit(5), FALSE))

## Commit without adding changes should produce an error
tools::assertError(commit(repo, "Test to commit"))

## Add another commit
writeLines(c("Hello world!", "HELLO WORLD!"), file.path(path, "test.txt"))
add(repo, "test.txt")
commit_2 <- commit(repo, "Commit message 2")
summary(commit_2)
tag_2 <- tag(repo, "Tagname2", "Tag message 2")

## Check relationship
stopifnot(identical(descendant_of(commit_2, commit_1), TRUE))
stopifnot(identical(descendant_of(commit_1, commit_2), FALSE))
stopifnot(identical(descendant_of(tag_2, tag_1), TRUE))
stopifnot(identical(descendant_of(tag_1, tag_2), FALSE))
stopifnot(identical(descendant_of(branches(repo)[[1]], commit_1), TRUE))
stopifnot(identical(descendant_of(commit_1, branches(repo)[[1]]), FALSE))
stopifnot(identical(length(parents(commit_2)), 1L))
stopifnot(identical(parents(commit_2)[[1]], commit_1))

## Check contributions
stopifnot(identical(
    colnames(contributions(repo, by = "author", breaks = "day")),
    c("when", "author", "n")))
stopifnot(identical(colnames(contributions(repo)),
                    c("when", "n")))
stopifnot(identical(nrow(contributions(repo)), 1L))
stopifnot(identical(contributions(repo)$n, 2L))
stopifnot(identical(contributions(repo, by = "author", breaks = "day")$n, 2L))

## Add another commit with 'all' argument
writeLines(c("Hello world!", "HELLO WORLD!", "HeLlO wOrLd!"),
           file.path(path, "test.txt"))
commit(repo, "Commit message 3", all = TRUE)

status_clean <- structure(list(staged = empty_named_list(),
                               unstaged = empty_named_list(),
                               untracked = empty_named_list()),
                          class = "git_status")
stopifnot(identical(status(repo), status_clean))

## Delete file and commit with 'all' argument
file.remove(file.path(path, "test.txt"))
commit(repo, "Commit message 4", all = TRUE)

stopifnot(identical(status(repo), status_clean))

## Add and commit multiple tracked files with 'all' argument
writeLines(sample(letters, 3), file.path(path, "test2.txt"))
add(repo, "test2.txt")
writeLines(sample(letters, 3), file.path(path, "test3.txt"))
add(repo, "test3.txt")
writeLines(sample(letters, 3), file.path(path, "test4.txt"))
add(repo, "test4.txt")
commit(repo, "Commit message 5")

stopifnot(identical(status(repo), status_clean))

writeLines(sample(letters, 3), file.path(path, "test2.txt"))
writeLines(sample(letters, 3), file.path(path, "test3.txt"))
writeLines(sample(letters, 3), file.path(path, "test4.txt"))
commit(repo, "Commit message 6", all = TRUE)

stopifnot(identical(status(repo), status_clean))

## Add one tracked file and delete another with 'all' argument
writeLines(sample(letters, 3), file.path(path, "test2.txt"))
file.remove(file.path(path, "test4.txt"))
commit(repo, "Commit message 7", all = TRUE)

stopifnot(identical(status(repo), status_clean))

## Delete multiple tracked files with 'all' argument
file.remove(file.path(path, "test2.txt"))
file.remove(file.path(path, "test3.txt"))
commit(repo, "Commit message 8", all = TRUE)

stopifnot(identical(status(repo), status_clean))

## Check max number of commits in output
stopifnot(identical(length(commits(repo)), 8L))
stopifnot(identical(length(commits(repo, n = -1)), 8L))
stopifnot(identical(length(commits(repo, n = 2)), 2L))
tools::assertError(commits(repo, n = 2.2))
tools::assertError(commits(repo, n = "2"))
tools::assertError(commits(repo, n = 1:2))

## Check to coerce repository to data.frame
df <- as.data.frame(repo)
stopifnot(identical(dim(df), c(8L, 6L)))
stopifnot(identical(names(df), c("sha", "summary", "message",
                                 "author", "email", "when")))

## Set working directory to path and check commits
setwd(path)
stopifnot(identical(sha(last_commit()), sha(commits(repo, n = 1)[[1]])))
stopifnot(identical(length(commits()), 8L))
stopifnot(identical(length(commits(n = -1)), 8L))
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
stopifnot(length(grep("'commit' must be an S3 class git_commit",
                      res[[1]]$message)) > 0)
res <- tools::assertError(.Call(git2r:::git2r_commit_tree, 3))
stopifnot(length(grep("'commit' must be an S3 class git_commit",
                      res[[1]]$message)) > 0)
res <- tools::assertError(.Call(git2r:::git2r_commit_tree, repo))
stopifnot(length(grep("'commit' must be an S3 class git_commit",
                      res[[1]]$message)) > 0)
commit_1$sha <- NA_character_
res <- tools::assertError(.Call(git2r:::git2r_commit_tree, commit_1))
stopifnot(length(grep("'commit' must be an S3 class git_commit",
                      res[[1]]$message)) > 0)

## Cleanup
unlink(path, recursive = TRUE)

if (identical(Sys.getenv("NOT_CRAN"), "true") ||
    identical(Sys.getenv("R_COVR"), "true")) {
    path <- tempfile(pattern = "git2r-")
    dir.create(path)
    setwd(path)
    system("git clone --depth 2 https://github.com/ropensci/git2r.git")

    ## Check the number of commits in the shallow clone.
    stopifnot(identical(length(commits(repository("git2r"))), 2L))
    stopifnot(identical(length(commits(repository("git2r"), n = 1)), 1L))

    ## Cleanup
    unlink(path, recursive = TRUE)
}
