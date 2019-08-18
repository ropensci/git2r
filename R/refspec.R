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

##' Create push refspec from arguments
##'
##' @param repo a \code{git_repository} object.
##' @param name The remote's name. Default is NULL.
##' @param refspec The refspec to be pushed. Default is NULL.
##' @param opts List with push options. Default is NULL.
##' @return List with remote (character vector) and refspec (character
##' vector).
##' @noRd
get_refspec <- function(repo = NULL, remote = NULL, spec = NULL, opts = NULL) {
    stopifnot(inherits(repo, "git_repository"))

    if (is_detached(repo))
        stop("You are not currently on a branch.")

    ## Options:
    if (!is.null(opts)) {
        stopifnot(is.list(opts))
    } else {
        opts <- list()
    }

    ## Remote:
    ## From: http://git-scm.com/docs/git-push
    ## When the command line does not specify where to push with the
    ## <repository> argument, branch.*.remote configuration for the
    ## current branch is consulted to determine where to push. If the
    ## configuration is missing, it defaults to origin.
    if (!is.null(remote)) {
        stopifnot(is.character(remote), identical(length(remote), 1L))
        remote <- sub("^[[:space:]]*", "", sub("[[:space:]]*$", "", remote))
        if (identical(nchar(remote), 0L))
            remote <- NULL
    }
    if (is.null(remote)) {
        remote <- .Call(git2r_config_get_string,
                        repo,
                        paste0("branch.",
                               repository_head(repo)$name,
                               ".remote"))
        if (is.null(remote))
            remote <- "origin"
    }

    ## Refspec:
    stopifnot(is.character(spec))

    if (isTRUE(opts$force))
        spec <- paste0("+", spec)

    list(remote = remote, refspec = spec)
}
