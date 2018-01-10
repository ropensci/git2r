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

##' Config
##'
##' Config file management. To display the configuration variables,
##' call method \code{config} without the \code{user.name},
##' \code{user.email} or \code{...} options.
##'
##' There are two ways git2r can find the local repository when
##' writing local options (1) Use the \code{repo} argument. (2) If the
##' \code{repo} argument is \code{NULL} but the current working
##' directory is inside the local repository, then \code{git2r} uses
##' that repository.
##' @param repo The \code{repository}. Default is NULL.
##' @param global Write option(s) to global configuration
##' file. Default is FALSE.
##' @param user.name The user name. Use NULL to delete the entry
##' @param user.email The e-mail address. Use NULL to delete the entry
##' @param ... Additional options to write or delete from the
##' configuration.
##' @return S3 class \code{git_config}. When writing options, the
##' configuration is returned invisible.
##' @export
##' @examples \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern = "git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Set user name and email.
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Display configuration
##' config(repo)
##'
##' ## Delete user email.
##' config(repo, user.email = NULL)
##'
##' ## Display configuration
##' config(repo)
##' }
config <- function(repo = NULL, global = FALSE, user.name, user.email, ...)
{
    if (is.null(repo)) {
        repo <- discover_repository(getwd())
        if (!is.null(repo))
            repo <- repository(repo)
    }

    ## Check that 'global' is either TRUE or FALSE
    stopifnot(any(identical(global, TRUE), identical(global, FALSE)))

    variables <- list(...)
    if (!missing(user.name))
        variables <- c(variables, list(user.name = user.name))

    if (!missing(user.email))
        variables <- c(variables, list(user.email = user.email))

    if (length(variables)) {
        for (i in seq_len(length(variables))) {
            if (!is.null(variables[[i]])) {
                if (!is.character(variables[[i]])) {
                    stop("'", names(variables)[i], "' must be a character vector")
                }
            }
        }

        if (isTRUE(global)) {
            repo <- NULL
        } else if (is.null(repo)) {
            stop("Unable to locate local repository")
        }

        .Call(git2r_config_set, repo, variables)
    }

    cfg <- .Call(git2r_config_get, repo)

    ## Sort the variables within levels by name
    cfg <- structure(lapply(cfg, function(x) x[order(names(x))]),
                     class = "git_config")

    if (length(variables)) {
        invisible(cfg)
    } else {
        return(cfg)
    }
}

##' @export
print.git_config <- function(x, ...) {
    lapply(names(x), function(level) {
        cat(sprintf("%s:\n", level))
        lapply(names(x[[level]]), function(entry) {
            cat(sprintf("        %s=%s\n", entry, x[[level]][[entry]][1]))
        })
    })
}
