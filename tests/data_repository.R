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

# currently odb_blobs() can't handle subsecond commits
# when TRUE Sys.sleep(1) is added before each commit
subsecond <- TRUE

## Initialize a repository
data_repo <- init(path)
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
        write_delim_git(x, "test.txt", data_repo, stage = TRUE)
    )[[1]]$message,
    "file extensions are stripped"
))
z <- status(data_repo)
stopifnot(
    all.equal(z$staged, list(new = "test.tsv", new = "test.yml"))
)
write_delim_git(x, "test", data_repo)
stopifnot(all.equal(status(data_repo), z))
add(data_repo, path = ".")
if (subsecond) Sys.sleep(1)
commit_1 <- commit(data_repo, "initial commit")

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
stopifnot(all.equal(
    tools::assertWarning(
        write_delim_git(x, "test", data_repo, sorting = "y", override = TRUE)
    )[[1]]$message,
"sorting results in ties. Add extra sorting variables to ensure small diffs."
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
add(data_repo, path = ".")
if (subsecond) Sys.sleep(1)
commit_2 <- commit(data_repo, "test")

write_delim_git(x, "junk/test", data_repo)
add(data_repo, path = ".")
if (subsecond) Sys.sleep(1)
commit_3 <- commit(data_repo, "test")

rm_data(data_repo, ".", "tsv")
stopifnot(
    all.equal(
        status(data_repo)$unstaged,
        list(deleted = "junk/test.tsv", deleted = "test.tsv")
    )
)
write_delim_git(x, "junk/test", data_repo)
rm_data(data_repo, ".", "yml")
stopifnot(
    all.equal(
        status(data_repo)$unstaged,
        list(deleted = "test.tsv", deleted = "test.yml")
    )
)
rm_data(data_repo, ".", "both", stage = TRUE)
stopifnot(
    all.equal(
        status(data_repo)$unstaged,
        list(deleted = "test.tsv", deleted = "test.yml")
    )
)
stopifnot(
    all.equal(
        status(data_repo)$staged,
        list(
            deleted = "junk/test.tsv", deleted = "junk/test.yml"
        )
    )
)

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
if (subsecond) Sys.sleep(1)
commit_4 <- commit(data_repo, "more")

y <- x
y$logic <- sample(c(TRUE, FALSE, NA), replace = TRUE, size = nrow(y))
y$complex <- complex(real = rnorm(nrow(y)), imaginary = rnorm(nrow(y)))
y$timestamp <- seq(
    as.POSIXct("1900-01-01"),
    as.POSIXct("2050-01-01"),
    length = 26
)
y$date <- seq(
    as.Date("1970-01-01"),
    as.Date("1970-01-26"),
    length = 26
)
write_delim_git(y, "logical", data_repo, sorting = c("y", "logic"))
z <- read_delim_git("logical", data_repo)
y.sorted <- y[do.call(order, y[c("y", "logic")]), colnames(z)]
rownames(y.sorted) <- NULL
stopifnot(all.equal(y.sorted, z))
add(data_repo, path = ".")
if (subsecond) Sys.sleep(1)
commit_5 <- commit(data_repo, "logical")

stopifnot(all.equal(
    tools::assertError(
        write_delim_git(
            y, "logical", data_repo, sorting = c("y", "logic"), optimize = FALSE
        )
    )[[1]][["message"]],
    "old data was stored optimized"
))

write_delim_git(y, "verbose", data_repo, optimize = FALSE)
z <- read_delim_git("verbose", data_repo)
stopifnot(all.equal(y, z))

stopifnot(all.equal(
    tools::assertError(
        write_delim_git(y, "verbose", data_repo, optimize = TRUE)
    )[[1]][["message"]],
    "old data was stored verbose"
))
add(data_repo, path = ".")
if (subsecond) Sys.sleep(1)
commit_6 <- commit(data_repo, "verbose")

yml <- file.path(path, "verbose.yml")
meta <- head(readLines(yml), -1)
writeLines(text = meta, con = yml)
add(data_repo, path = ".")
commit_7 <- commit(data_repo, "fast")
stopifnot(all.equal(
    tools::assertError(
        read_delim_git("verbose", data_repo)
    )[[1]][["message"]],
    "error in metadata"
))
stopifnot(all.equal(
    tools::assertError(
        write_delim_git(y, "verbose", data_repo, optimize = FALSE)
    )[[1]][["message"]],
    "error in existing metadata"
))
stopifnot(all.equal(
    tools::assertError(
        rm_data(path)
    )[[1]][["message"]],
    "'path' must be a character vector"
))
stopifnot(all.equal(
    tools::assertError(
        rm_data(path, c(".", "junk"))
    )[[1]][["message"]],
    "'path' must be a single value"
))

com <- recent_commit(data_repo, "test.tsv")
stopifnot(inherits(com, "data.frame"))
stopifnot(all.equal(colnames(com), c("commit", "author", "when")))
stopifnot(all.equal(com$commit, commit_2$sha))

com <- recent_commit(data_repo, "test", data = TRUE)
stopifnot(inherits(com, "data.frame"))
stopifnot(all.equal(colnames(com), c("commit", "author", "when")))
stopifnot(all.equal(com$commit, commit_2$sha))

com <- recent_commit(data_repo, "junk/test", data = TRUE)
stopifnot(all.equal(com$commit, commit_3$sha))
com <- recent_commit(data_repo, "junk/test.tsv")
stopifnot(all.equal(com$commit, commit_3$sha))
com <- recent_commit(data_repo, "junk/test.yml")
stopifnot(all.equal(com$commit, commit_3$sha))

stopifnot(all.equal(
    tools::assertError(
        recent_commit(data_repo, TRUE)
    )[[1]][["message"]],
    "'path' must be a character vector"
))
stopifnot(all.equal(
    tools::assertError(
        recent_commit(data_repo, c("junk", "test"))
    )[[1]][["message"]],
    "'path' must be a single value"
))

stopifnot(all.equal(
    tools::assertWarning(
        com <- recent_commit(data_repo, "verbose.yml")
    )[[1]][["message"]],
    "Multiple commits within the same second"
))
stopifnot(nrow(com) == 2)
stopifnot(all(com$commit %in% c(commit_6$sha, commit_7$sha)))
