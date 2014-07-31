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

##' Display status
##' @keywords internal
display_status <- function(title, section) {
    cat(sprintf("%s:\n", title))

    for(i in seq_len(length(section))) {
        label <- names(section)[i]
        label <- paste0(toupper(substr(label, 1, 1)),
                        substr(label, 2, nchar(label)))
        cat(paste0("\t", label, ":   ", section[[i]], "\n"))
    }

    invisible(NULL)
}

##' Status
##'
##' Display state of the repository working directory and the staging
##' area.
##' @rdname status-methods
##' @docType methods
##' @param repo The \code{git_repository} to get status from.
##' @param staged Include staged files. Default TRUE.
##' @param unstaged Include unstaged files. Default TRUE.
##' @param untracked Include untracked files. Default TRUE.
##' @param ignored Include ignored files. Default FALSE.
##' @param verbose Display status. Default TRUE.
##' @return invisible(list) with repository status
##' @keywords methods
##' @include repository.r
##' @examples \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' status(repo)
##'}
setGeneric("status",
           signature = "repo",
           function(repo,
                    staged    = TRUE,
                    unstaged  = TRUE,
                    untracked = TRUE,
                    ignored   = FALSE,
                    verbose   = TRUE)
           standardGeneric("status"))

##' @rdname status-methods
##' @export
setMethod("status",
          signature(repo = "git_repository"),
          function (repo, staged, unstaged, untracked, ignored, verbose)
          {
              s <- .Call("git2r_status_list",
                         repo,
                         staged,
                         unstaged,
                         untracked,
                         ignored)

              if (verbose) {
                  if (length(s$ignored)) {
                      display_status("Ignored files", s$ignored)
                      cat("\n")
                  }

                  if (length(s$untracked)) {
                      display_status("Untracked files", s$untracked)
                      cat("\n")
                  }

                  if (length(s$unstaged)) {
                      display_status("Unstaged changes", s$unstaged)
                      cat("\n")
                  }

                  if (length(s$staged)) {
                      display_status("Staged changes", s$staged)
                      cat("\n")
                  }
              }

              invisible(s)
          }
)
