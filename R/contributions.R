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

##' Contributions
##'
##' See contributions to a Git repo
##' @template repo-param
##' @param breaks Default is \code{month}. Change to year, quarter,
##' week or day as necessary.
##' @param by Contributions by "commits" or "author". Default is "commits".
##' @return A \code{data.frame} with contributions.
##' @export
##' @examples
##' \dontrun{
##' ## Create directories and initialize repositories
##' path_bare <- tempfile(pattern="git2r-")
##' path_repo_1 <- tempfile(pattern="git2r-")
##' path_repo_2 <- tempfile(pattern="git2r-")
##' dir.create(path_bare)
##' dir.create(path_repo_1)
##' dir.create(path_repo_2)
##' repo_bare <- init(path_bare, bare = TRUE)
##'
##' ## Clone to repo 1 and config user
##' repo_1 <- clone(path_bare, path_repo_1)
##' config(repo_1, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Add changes to repo 1 and push to bare
##' lines <- "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do"
##' writeLines(lines, file.path(path_repo_1, "test.txt"))
##' add(repo_1, "test.txt")
##' commit(repo_1, "First commit message")
##'
##' ## Add more changes to repo 1
##' lines <- c(
##'   "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'   "eiusmod tempor incididunt ut labore et dolore magna aliqua.")
##' writeLines(lines, file.path(path_repo_1, "test.txt"))
##' add(repo_1, "test.txt")
##' commit(repo_1, "Second commit message")
##'
##' ## Push to bare
##' push(repo_1, "origin", "refs/heads/master")
##'
##' ## Clone to repo 2
##' repo_2 <- clone(path_bare, path_repo_2)
##' config(repo_2, user.name = "Bob", user.email = "bob@@example.org")
##'
##' ## Add changes to repo 2
##' lines <- c(
##'   "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'   "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad",
##'   "minim veniam, quis nostrud exercitation ullamco laboris nisi ut")
##' writeLines(lines, file.path(path_repo_2, "test.txt"))
##' add(repo_2, "test.txt")
##' commit(repo_2, "Third commit message")
##'
##' ## Push to bare
##' push(repo_2, "origin", "refs/heads/master")
##'
##' ## Pull changes to repo 1
##' pull(repo_1)
##'
##' ## View contributions by day
##' contributions(repo_1)
##'
##' ## View contributions by author and day
##' contributions(repo_1, by = "author")
##' }
contributions <- function(repo = ".",
                          breaks = c("month", "year", "quarter", "week", "day"),
                          by = c("commits", "author")) {
    breaks <- match.arg(breaks)
    by <- match.arg(by)

    ctbs <- .Call(git2r_revwalk_contributions, lookup_repository(repo),
                  TRUE, TRUE, FALSE)
    ctbs$when <- as.POSIXct(ctbs$when, origin = "1970-01-01", tz = "GMT")
    ctbs$when <- as.POSIXct(cut(ctbs$when, breaks = breaks))

    if (identical(by, "commits")) {
        ctbs <- as.data.frame(table(ctbs$when))
        names(ctbs) <- c("when", "n")
        ctbs$when <- as.Date(ctbs$when)
    } else {
        ## Create an index and tabulate
        ctbs$index <- paste0(ctbs$when, ctbs$author, ctbs$email)
        count <- as.data.frame(table(ctbs$index),
                               stringsAsFactors = FALSE)
        names(count) <- c("index", "n")

        ## Match counts and clean result
        ctbs <- as.data.frame(ctbs)
        ctbs$n <- count$n[match(ctbs$index, count$index)]
        ctbs <- unique(ctbs[, c("when", "author", "n")])
        ctbs$when <- as.Date(substr(as.character(ctbs$when), 1, 10))
        ctbs <- ctbs[order(ctbs$when, ctbs$author), ]
        row.names(ctbs) <- NULL
    }

    ctbs
}
