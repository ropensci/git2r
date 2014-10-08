## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2014 The git2r contributors
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

## Check validity of S4 class git_tag
## Each slot must have length equal to one
when <- new("git_time", time = 1395567947, offset = 60)
tagger <- new("git_signature",
              name = "Repo",
              email = "repo@example.org",
              when = when)

tools::assertError(validObject(new("git_tag",
                                   message = character(0),
                                   name = "name1",
                                   tagger = tagger,
                                   target = "target1")))

tools::assertError(validObject(new("git_tag",
                                   message = c("message1", "message2"),
                                   name = "name1",
                                   tagger = tagger,
                                   target = "target1")))

tools::assertError(validObject(new("git_tag",
                                   message = "message1",
                                   name = character(0),
                                   tagger = tagger,
                                   target = "target1")))

tools::assertError(validObject(new("git_tag",
                                   message = "message1",
                                   name = c("name1", "name2"),
                                   tagger = tagger,
                                   target = "target1")))

tools::assertError(validObject(new("git_tag",
                                   message = "message1",
                                   name = "name1",
                                   tagger = tagger,
                                   target = character(0))))

tools::assertError(validObject(new("git_tag",
                                   message = "message1",
                                   name = "name1",
                                   tagger = tagger,
                                   target = c("target1", "target2"))))

## Create a directory in tempdir
path <- tempfile(pattern="git2r-")
dir.create(path)

## Initialize a repository
repo <- init(path)
config(repo, user.name="Repo", user.email="repo@example.org")

## Create a file
writeLines("Hello world!", file.path(path, "test.txt"))

## add and commit
add(repo, 'test.txt')
commit(repo, "Commit message")

## Check tags, no tag added
stopifnot(identical(tags(repo), list()))

## Create tag
new_tag <- tag(repo, "Tagname", "Tag message")

## Check tag
stopifnot(identical(lookup(repo, new_tag@sha), new_tag))
stopifnot(identical(new_tag@name, "Tagname"))
stopifnot(identical(new_tag@message, "Tag message"))
stopifnot(identical(new_tag@tagger@name, "Repo"))
stopifnot(identical(new_tag@tagger@email, "repo@example.org"))
stopifnot(identical(length(tags(repo)), 1L))
stopifnot(identical(tags(repo)[[1]]@name, "Tagname"))
stopifnot(identical(tags(repo)[[1]]@message, "Tag message"))
stopifnot(identical(tags(repo)[[1]]@tagger@name, "Repo"))
stopifnot(identical(tags(repo)[[1]]@tagger@email, "repo@example.org"))

## Check objects in object database
stopifnot(identical(table(odb_objects(repo)$type),
                    structure(c(1L, 1L, 1L, 1L),
                              .Dim = 4L,
                              .Dimnames = structure(list(
                                  c("blob", "commit", "tag", "tree")),
                                  .Names = ""),
                              class = "table")))

## Cleanup
unlink(path, recursive=TRUE)
