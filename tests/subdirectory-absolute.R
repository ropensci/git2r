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

## Testing use of absolute paths when the working directory is a subdirectory

library("git2r")

## For debugging
sessionInfo()

## Initialize a temporary repository
path <- tempfile(pattern = "git2r-")
dir.create(path)
dir.create(file.path(path, "subfolder"))
repo <- init(path)

## Create a user
config(repo, user.name = "Alice", user.email = "alice@example.org")

# Change working directory to subdirectory
cwd <- setwd(file.path(path,  "subfolder"))

## Create two files
root <- file.path(path, "root.txt")
sub <- file.path(path, "subfolder", "sub.txt")
writeLines("root file",  root)
writeLines("sub file", sub)

## Add and commit
add(repo, c(root, sub))
commit(repo, "Add files")

## Check commits
commits_root <- commits(repo, path = root)
stopifnot(length(commits_root) == 1)
commits_sub <- commits(repo, path = sub)
stopifnot(length(commits_sub) == 1)
stopifnot(identical(commits_root, commits_sub))

## Remove and commit
rm_file(repo, c(root, sub))
commit(repo, "Remove files")

## Check commits
commits_total <- commits(repo)
stopifnot(length(commits_total) == 2)
commits_root_rm <- commits(repo, path = root)
stopifnot(length(commits_root_rm) == 2)
commits_sub_rm <- commits(repo, path = sub)
stopifnot(length(commits_sub_rm) == 2)
stopifnot(identical(commits_root_rm, commits_sub_rm))

## Cleanup
setwd(cwd)
unlink(path, recursive = TRUE)
