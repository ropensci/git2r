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
config <- function(repo = NULL, global = FALSE, user.name, user.email, ...) {
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
                    stop("'", names(variables)[i],
                         "' must be a character vector")
                }
            }
        }

        if (isTRUE(global)) {
            repo <- NULL
            if (.Platform$OS.type == "windows") {
                ## Ensure that git2r writes the config file to the
                ## root of the user's home directory by first creating
                ## an empty file. Otherwise it may be written to the
                ## user's Documents/ directory. Only create the empty
                ## file if the user has specified configuration
                ## options to set and no global config file exists.
                if (is.na(git_config_files()[["path"]][3])) {
                    if (length(variables) > 0) {
                        file.create(file.path(home_dir(), ".gitconfig"))
                    }
                }
            }
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

    invisible(x)
}

##' Locate the path to configuration files
##'
##' Potential configuration files:
##' \describe{
##'   \item{system}{
##'     Locate the path to the system configuration file. If
##'     '/etc/gitconfig' doesn't exist, it will look for
##'     '\%PROGRAMFILES\%'.
##'   }
##'   \item{xdg}{
##'     Locate the path to the global xdg compatible configuration
##'     file. The xdg compatible configuration file is usually located
##'     in '$HOME/.config/git/config'. This method will try to guess
##'     the full path to that file, if the file exists.
##'   }
##'   \item{global}{
##'     The user or global configuration file is usually located in
##'     '$HOME/.gitconfig'. This method will try to guess the full
##'     path to that file, if the file exists.
##'   }
##'   \item{local}{
##'     Locate the path to the repository specific configuration file,
##'     if the file exists.
##'   }
##' }
##' @template repo-param
##' @return a \code{data.frame} with one row per potential
##'     configuration file where \code{NA} means not found.
##' @export
git_config_files <- function(repo = ".") {
    ## Lookup repository
    if (inherits(repo, "git_repository")) {
        repo <- repo$path
    } else if (is.null(repo)) {
        repo <- discover_repository(getwd())
    } else if (is.character(repo) && (length(repo) == 1) &&
               !is.na(repo) && isTRUE(file.info(repo)$isdir)) {
        repo <- discover_repository(repo)
    } else {
        repo <- NULL
    }

    ## Find local configuration file
    if (is.null(repo)) {
        path <- NA_character_
    } else {
        path <- file.path(normalizePath(repo), "config")
        if (!isTRUE(!file.info(path)$isdir))
            path <- NA_character_
    }

    data.frame(file = c("system", "xdg", "global", "local"),
               path = c(.Call(git2r_config_find_file, "system"),
                        .Call(git2r_config_find_file, "xdg"),
                        .Call(git2r_config_find_file, "global"),
                        path),
               stringsAsFactors = FALSE)
}
