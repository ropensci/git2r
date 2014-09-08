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

##
## Create a directory in tempdir
##
path <- tempfile(pattern="git2r-")
dir.create(path)

##
## Initialize a repository
##
repo <- init(path)
config(repo, user.name="Repo", user.email="repo@example.org")

##
## Create a file
##
writeLines("Hello world!", file.path(path, "test.txt"))

##
## add and commit
##
add(repo, "test.txt")
new_commit <- commit(repo, "Commit message")

##
## Lookup blob
##
blob <- lookup(repo, "cd0875583aabe89ee197ea133980a9085d08e497")
stopifnot(is(blob, "git_blob"))
stopifnot(identical(is_blob(blob), TRUE))
stopifnot(identical(is_binary(blob), FALSE))
stopifnot(identical(blob, lookup(repo, "cd0875")))
stopifnot(identical(length(blob), 13L))
stopifnot(identical(content(blob), "Hello world!"))

##
## Add one more commit
##
writeLines(c("Hello world!", "HELLO WORLD!", "HeLlO wOrLd!"),
           file.path(path, "test.txt"))
add(repo, "test.txt")
blob <- tree(commit(repo, "New commit message"))[1]
stopifnot(identical(content(blob),
                    c("Hello world!", "HELLO WORLD!", "HeLlO wOrLd!")))

##
## Hash
##
stopifnot(identical(hash("Hello, world!\n"),
                    "af5626b4a114abcb82d63db7c8082c3c4756e51b"))
stopifnot(identical(hash("test content\n"),
                    "d670460b4b4aece5915caf5c68d12f560a9fe3e4"))
stopifnot(identical(hash(c("Hello, world!\n",
                           "test content\n")),
                    c("af5626b4a114abcb82d63db7c8082c3c4756e51b",
                      "d670460b4b4aece5915caf5c68d12f560a9fe3e4")))
stopifnot(identical(hash(c("Hello, world!\n",
                           NA_character_,
                           "test content\n")),
                    c("af5626b4a114abcb82d63db7c8082c3c4756e51b",
                      NA_character_,
                      "d670460b4b4aece5915caf5c68d12f560a9fe3e4")))
stopifnot(identical(hash(character(0)), character(0)))

##
## Hash file
##
test_1_txt <- file(file.path(path, "test-1.txt"), "wb")
writeChar("Hello, world!\n", test_1_txt, eos = NULL)
close(test_1_txt)
test_2_txt <- file(file.path(path, "test-2.txt"), "wb")
writeChar("test content\n", test_2_txt, eos = NULL)
close(test_2_txt)
stopifnot(identical(hash("Hello, world!\n"),
                    hashfile(file.path(path, "test-1.txt"))))
stopifnot(identical(hash("test content\n"),
                    hashfile(file.path(path, "test-2.txt"))))
stopifnot(identical(hash(c("Hello, world!\n",
                           "test content\n")),
                    hashfile(c(file.path(path, "test-1.txt"),
                               file.path(path, "test-2.txt")))))
tools::assertError(hashfile(c(file.path(path, "test-1.txt"),
                              NA_character_,
                              file.path(path, "test-2.txt"))))
stopifnot(identical(hashfile(character(0)), character(0)))

##
## Create blob from disk
##
tmp_file_1 <- tempfile()
tmp_file_2 <- tempfile()
writeLines("Hello, world!", tmp_file_1)
writeLines("test content", tmp_file_2)
blob_list_1 <- blob_create(repo, c(tmp_file_1, tmp_file_2), relative = FALSE)
unlink(tmp_file_1)
unlink(tmp_file_2)
stopifnot(identical(sapply(blob_list_1, slot, "sha"),
                    c("af5626b4a114abcb82d63db7c8082c3c4756e51b",
                      "d670460b4b4aece5915caf5c68d12f560a9fe3e4")))

##
## Create blob from workdir
##
writeLines("Hello, world!", file.path(path, "test-workdir-1.txt"))
writeLines("test content", file.path(path, "test-workdir-2.txt"))
blob_list_2 <- blob_create(repo, c("test-workdir-1.txt",
                                   "test-workdir-2.txt"))
stopifnot(identical(sapply(blob_list_2, slot, "sha"),
                    c("af5626b4a114abcb82d63db7c8082c3c4756e51b",
                      "d670460b4b4aece5915caf5c68d12f560a9fe3e4")))

##
## Cleanup
##
unlink(path, recursive=TRUE)
