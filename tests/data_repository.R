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

stopifnot(all.equal(
    tools::assertWarning(
        write_delim_git(x, "test.txt", data_repo)
    )[[1]]$message,
    "file extensions are stripped"
))
z <- status(data_repo)
print(z)
stopifnot(
    all.equal(z$s, list(new = "test.tsv", new = "test.yml"))
)
write_delim_git(x, "test", data_repo)
stopifnot(all.equal(status(data_repo), z))
stopifnot(all.equal(
    tools::assertError(
        write_delim_git(x[, 1:3], "test", data_repo)
    )[[1]][["message"]],
    "new data lacks old sorting variable, use override = TRUE"
))
y <- x
y$junk <- x$x
stopifnot(all.equal(
    tools::assertError(
        write_delim_git(y, "test", data_repo)
    )[[1]][["message"]],
    "old data has different number of variables, use override = TRUE"
))
y <- x
y$x <- factor(y$x)
stopifnot(all.equal(
    tools::assertError(
        write_delim_git(y, "test", data_repo)
    )[[1]][["message"]],
    "old data has different variable types or sorting, use override = TRUE"
))
stopifnot(all.equal(
    x,
    read_delim_git("test", data_repo)
))
write_delim_git(x, "test", data_repo, sorting = c("y", "x"), override = TRUE)
x_sorted <- x[do.call(order, x[c("y", "x")]), c("y", "x", "z", "abc")]
rownames(x_sorted) <- NULL
stopifnot(all.equal(x_sorted, read_delim_git("test", data_repo)))
y <- x
y$abc <- NULL
y$xyz <- x$abc
stopifnot(all.equal(
    tools::assertError(
        write_delim_git(y, "test", data_repo)
    )[[1]][["message"]],
    "old data has different variables, use override = TRUE"
))
y <- x
y$x[1] <- NA
stopifnot(
    grepl(
        "^The string 'NA' cannot be stored",
        tools::assertError(
            write_delim_git(y, "test", data_repo)
        )[[1]]["message"]
    )
)


stopifnot(all.equal(
    tools::assertError(read_delim_git("", data_repo))[[1]][["message"]],
    "raw file and/or meta file missing"
))
stopifnot(all.equal(
    tools::assertError(read_delim_git("test", "."))[[1]][["message"]],
    "repo is not a 'data_repository'"
))

write_delim_git(x, "junk/test", data_repo)
commit(data_repo, "test")
rm_file(data_repo, ".tsv")
stopifnot(
    all.equal(
        status(data_repo)$s,
        list(deleted = "junk/test.tsv", deleted = "test.tsv")
    )
)
write_delim_git(x, "junk/test", data_repo)
rm_file(data_repo, ".yml")
stopifnot(
    all.equal(
        status(data_repo)$s,
        list(deleted = "test.tsv", deleted = "test.yml")
    )
)
rm_file(data_repo, "junk/test")
stopifnot(
    all.equal(
        status(data_repo)$s,
        list(
            deleted = "junk/test.tsv", deleted = "junk/test.yml",
            deleted = "test.tsv", deleted = "test.yml"
        )
    )
)

stopifnot(all.equal(
    tools::assertError(
        write_delim_git(x, "test", repository(path))
    )[[1]][["message"]],
    "repo is not a 'data_repository'"
))
stopifnot(all.equal(
    tools::assertError(
        write_delim_git(x, "test", sorting = character(0), data_repo)
    )[[1]][["message"]],
    "at least one variable is required for sorting"
))
stopifnot(all.equal(
    tools::assertError(
        write_delim_git(x, "test", sorting = "junk", data_repo)
    )[[1]][["message"]],
    "use only variables of 'x' for sorting"
))

y <- x
y$logic <- sample(c(TRUE, FALSE, NA), replace = TRUE, size = nrow(y))
y$complex <- complex(real = rnorm(nrow(y)), imaginary = rnorm(nrow(y)))
y$timestamp <- seq(
    as.POSIXct("1900-01-01"),
    as.POSIXct("2050-01-01"),
    length = 26
)
write_delim_git(y, "logical", data_repo, sorting = c("y", "logic"))
z <- read_delim_git("logical", data_repo)
y.sorted <- y[do.call(order, y[c("y", "logic")]), colnames(z)]
rownames(y.sorted) <- NULL
stopifnot(all.equal(y.sorted, z))
