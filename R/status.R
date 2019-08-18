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

##' Status
##'
##' Display state of the repository working directory and the staging
##' area.
##' @template repo-param
##' @param staged Include staged files. Default TRUE.
##' @param unstaged Include unstaged files. Default TRUE.
##' @param untracked Include untracked files and directories. Default
##'     TRUE.
##' @param ignored Include ignored files. Default FALSE.
##' @param all_untracked Shows individual files in untracked
##'     directories if \code{untracked} is \code{TRUE}.
##' @return \code{git_status} with repository status
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Config user
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Create a file
##' writeLines("Hello world!", file.path(path, "test.txt"))
##'
##' ## Check status; untracked file
##' status(repo)
##'
##' ## Add file
##' add(repo, "test.txt")
##'
##' ## Check status; staged file
##' status(repo)
##'
##' ## Commit
##' commit(repo, "First commit message")
##'
##' ## Check status; clean
##' status(repo)
##'
##' ## Change the file
##' writeLines(c("Hello again!", "Here is a second line", "And a third"),
##'            file.path(path, "test.txt"))
##'
##' ## Check status; unstaged file
##' status(repo)
##'
##' ## Add file and commit
##' add(repo, "test.txt")
##' commit(repo, "Second commit message")
##'
##' ## Check status; clean
##' status(repo)
##'}
status <- function(repo      = ".",
                   staged    = TRUE,
                   unstaged  = TRUE,
                   untracked = TRUE,
                   ignored   = FALSE,
                   all_untracked = FALSE) {
    structure(.Call(git2r_status_list, lookup_repository(repo), staged,
                    unstaged, untracked, all_untracked, ignored),
              class = "git_status")
}

##' @export
print.git_status <- function(x, ...) {
    display_status <- function(title, section) {
        cat(sprintf("%s:\n", title))

        for (i in seq_len(length(section))) {
            label <- names(section)[i]
            label <- paste0(toupper(substr(label, 1, 1)),
                            substr(label, 2, nchar(label)))
            cat(sprintf("\t%-12s%s\n", paste0(label, ":"), section[[i]]))
        }

        invisible(NULL)
    }

    if (max(sapply(x, length)) == 0L)
      cat("working directory clean\n")

    if (length(x$ignored)) {
        display_status("Ignored files", x$ignored)
        cat("\n")
    }

    if (length(x$untracked)) {
        display_status("Untracked files", x$untracked)
        cat("\n")
    }

    if (length(x$unstaged)) {
        display_status("Unstaged changes", x$unstaged)
        cat("\n")
    }

    if (length(x$staged)) {
        display_status("Staged changes", x$staged)
        cat("\n")
    }

    invisible(x)
}
