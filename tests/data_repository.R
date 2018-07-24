## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2018 The git2r contributors
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

## Create a directory in tempdir
path <- tempfile(pattern = "git2r-")
dir.create(path)

## Initialize a data repository
data_repo <- init(path, project = "test")
stopifnot(inherits(data_repo, "git_repository"))
stopifnot(all.equal(data_repo$path, file.path(path, ".git")))
stopifnot(all.equal(data_repo$project, "test"))
config(data_repo, user.name = "Alice", user.email = "alice@example.org")

stopifnot(all.equal(
    tools::assertError(write_delim_git(NA, "test", data_repo))[[1]][["message"]],
    "x is not a 'data.frame'"
))

x <- data.frame(
    x = LETTERS,
    y = factor(
        sample(c("a", "b", NA), 26, replace = TRUE),
        levels = c("a", "b", "c")
    ),
    z = c(NA, 1:25),
    abc = c(rnorm(25), NA),
    stringsAsFactors = FALSE
)
tools::assertWarning(wdg <- write_delim_git(x, "test.txt", data_repo))
z <- status(data_repo)
print(z)
stopifnot(
    all.equal(z$s, list(new = "test.tsv", new = "test.yml"))
)
all.equal(
    x,
    read_delim_git("test", data_repo)
)
stopifnot(all.equal(
    tools::assertError(read_delim_git(NA, data_repo))[[1]][["message"]],
    "raw file and/or meta file missing"
))
write_delim_git(x, "junk/test", data_repo)
commit(data_repo, "test")
rm_file(data_repo, "junk/test")
stopifnot(
    all.equal(
        status(data_repo)$s,
        list(deleted = "junk/test.tsv", deleted = "junk/test.yml")
    )
)
rm_file(data_repo)
stopifnot(
    all.equal(
        status(data_repo)$s,
        list(
            deleted = "junk/test.tsv", deleted = "junk/test.yml",
            deleted = "test.tsv", deleted = "test.yml"
        )
    )
)
