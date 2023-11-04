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


## Initialize a temporary repository
path <- tempfile(pattern = "git2r-")
dir.create(path)
repo <- init(path, branch = "main")

## Create a user and commit a file
config(repo, user.name = "Author", user.email = "author@example.org")
writeLines(c("First line in file 1.", "Second line in file 1."),
           file.path(path, "example-1.txt"))
add(repo, "example-1.txt")
commit(repo, "First commit message")

## Create and add one more file
writeLines(c("First line in file 2.", "Second line in file 2."),
           file.path(path, "example-2.txt"))
add(repo, "example-2.txt")
commit(repo, "Second commit message")

## Create a new branch 'fix'
checkout(repo, "fix", create = TRUE)

## Update 'example-1.txt' (swap words in first line) and commit
writeLines(c("line First in file 1.", "Second line in file 1."),
           file.path(path, "example-1.txt"))
add(repo, "example-1.txt")
commit(repo, "Third commit message")

checkout(repo, "main")

## Update 'example-2.txt' (swap words in second line) and commit
writeLines(c("First line in file 2.", "line Second in file 2."),
           file.path(path, "example-2.txt"))
add(repo, "example-2.txt")
commit(repo, "Fourth commit message")

# Missing branch to merge with should throw an error
tools::assertError(merge(repo))

## Merge 'fix'
m <- merge(repo, "fix", TRUE, default_signature(repo))
stopifnot(identical(format(m), "Merge"))

## Merge 'fix' again
m <- merge(repo, "fix", TRUE, default_signature(repo))
stopifnot(identical(format(m), "Already up-to-date"))

## Check number of parents of each commit
stopifnot(identical(sapply(commits(repo), function(x) length(parents(x))),
                    c(2L, 1L, 1L, 1L, 0L)))

## Check that last commit is a merge
stopifnot(is_merge(last_commit(repo)))
summary(last_commit(repo))

## Check that metadata associated with merge is removed
stopifnot(!file.exists(file.path(path, ".git", "MERGE_HEAD")))

## Cleanup
unlink(path, recursive = TRUE)
