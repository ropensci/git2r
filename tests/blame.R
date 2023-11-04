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
repo <- init(path)
config(repo, user.name = "Alice", user.email = "alice@example.org")

## Create a file and commit
writeLines("Hello world!", file.path(path, "test.txt"))
add(repo, "test.txt")
commit_1 <- commit(repo, "First commit message")

## Create new user and change file
config(repo, user.name = "Bob", user.email = "bob@example.org")
writeLines(c("Hello world!", "HELLO WORLD!", "HOLA"),
           file.path(path, "test.txt"))
add(repo, "test.txt")
commit_2 <- commit(repo, "Second commit message")

## Check blame
b <- blame(repo, "test.txt")
stopifnot(identical(length(b$hunks), 2L))

## Hunk: 1
stopifnot(identical(b$hunks[[1]]$lines_in_hunk, 1L))
stopifnot(identical(b$hunks[[1]]$final_commit_id, sha(commit_1)))
stopifnot(identical(b$hunks[[1]]$final_start_line_number, 1L))
stopifnot(identical(b$hunks[[1]]$final_signature$name, "Alice"))
stopifnot(identical(b$hunks[[1]]$final_signature$email, "alice@example.org"))
stopifnot(identical(b$hunks[[1]]$orig_commit_id, sha(commit_1)))
stopifnot(identical(b$hunks[[1]]$orig_start_line_number, 1L))
stopifnot(identical(b$hunks[[1]]$orig_signature$name, "Alice"))
stopifnot(identical(b$hunks[[1]]$orig_signature$email, "alice@example.org"))
stopifnot(identical(b$hunks[[1]]$orig_path, "test.txt"))
stopifnot(identical(b$hunks[[1]]$boundary, TRUE))

## Hunk: 2
stopifnot(identical(b$hunks[[2]]$lines_in_hunk, 2L))
stopifnot(identical(b$hunks[[2]]$final_commit_id, sha(commit_2)))
stopifnot(identical(b$hunks[[2]]$final_start_line_number, 2L))
stopifnot(identical(b$hunks[[2]]$final_signature$name, "Bob"))
stopifnot(identical(b$hunks[[2]]$final_signature$email, "bob@example.org"))
stopifnot(identical(b$hunks[[2]]$orig_commit_id, sha(commit_2)))
stopifnot(identical(b$hunks[[2]]$orig_start_line_number, 2L))
stopifnot(identical(b$hunks[[2]]$orig_signature$name, "Bob"))
stopifnot(identical(b$hunks[[2]]$orig_signature$email, "bob@example.org"))
stopifnot(identical(b$hunks[[2]]$orig_path, "test.txt"))
stopifnot(identical(b$hunks[[2]]$boundary, FALSE))

## Cleanup
unlink(path, recursive = TRUE)
