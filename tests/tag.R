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

## Create a file
writeLines("Hello world!", file.path(path, "test.txt"))

## add and commit
add(repo, "test.txt")
commit(repo, "Commit message")

## Check tags, no tag added
stopifnot(identical(tags(repo), empty_named_list()))

## Create tag
new_tag <- tag(repo, "Tagname", "Tag message")
stopifnot(identical(print(new_tag), new_tag))
summary(new_tag)

## Check tag
stopifnot(identical(lookup(repo, sha(new_tag)), new_tag))
stopifnot(identical(new_tag$name, "Tagname"))
stopifnot(identical(new_tag$message, "Tag message"))
stopifnot(identical(new_tag$tagger$name, "Alice"))
stopifnot(identical(new_tag$tagger$email, "alice@example.org"))
stopifnot(identical(length(tags(repo)), 1L))
stopifnot(identical(tags(repo)[[1]]$name, "Tagname"))
stopifnot(identical(tags(repo)[[1]]$message, "Tag message"))
stopifnot(identical(tags(repo)[[1]]$tagger$name, "Alice"))
stopifnot(identical(tags(repo)[[1]]$tagger$email, "alice@example.org"))

## Check objects in object database
stopifnot(identical(table(odb_objects(repo)$type),
                    structure(c(1L, 1L, 1L, 1L),
                              .Dim = 4L,
                              .Dimnames = structure(list(
                                  c("blob", "commit", "tag", "tree")),
                                  .Names = ""),
                              class = "table")))

## Delete tag
tag_delete(new_tag)
stopifnot(identical(length(tags(repo)), 0L))

## Create tag with session info
tag(repo, "Tagname", "Tag message", session = TRUE)
stopifnot(grep("git2r", tags(repo)[[1]]$message) > 0)

## Check tags method with default repo argument
wd <- setwd(path)
stopifnot(identical(length(tags()), 1L))
tag_delete(name = "Tagname")
stopifnot(identical(length(tags()), 0L))
if (!is.null(wd))
    setwd(wd)

## Cleanup
unlink(path, recursive = TRUE)
