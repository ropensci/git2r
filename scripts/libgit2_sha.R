library("git2r")

lines <- readLines("R/libgit2.R")
i <- grep("libgit2_sha <- function[(][)] [\"][0-9a-fA-F]{40}[\"]", lines)
stopifnot(identical(length(i), 1L))

sha <- last_commit(repository("../libgit2"))$sha
lines <- sub("libgit2_sha <- function[(][)] [\"][0-9a-fA-F]{40}[\"]",
             sprintf("libgit2_sha <- function() \"%s\"", sha),
             lines)

writeLines(lines, "R/libgit2.R")
