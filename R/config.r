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

##' Config
##'
##' Config file management. To display the configuration variables,
##' call method \code{config} with only the \code{repo} argument.
##' @rdname config-methods
##' @docType methods
##' @param repo the \code{repo} to configure
##' @param user.name the user name. Use NULL to delete the entry
##' @param user.email the e-mail address. Use NULL to delete the entry
##' @return An invisible \code{list} with the configuration
##' @keywords methods
##' @include repository.r
##' @examples \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Set user name and email. The configuration is returned
##' cfg <-config(repo, user.name="Repo", user.email="repo@@example.org")
##'
##' ## View configuration list
##' cfg
##'
##' ## Display configuration
##' config(repo)
##'}
setGeneric("config",
           signature = "repo",
           function(repo,
                    user.name,
                    user.email)
           standardGeneric("config"))

##' @rdname config-methods
##' @export
setMethod("config",
          signature(repo = "git_repository"),
          function(repo,
                   user.name,
                   user.email)
          {
              variables <- as.list(match.call(expand.dots = TRUE))
              variables <- variables[-(1:2)]

              if(length(variables)) {
                  ## Check that the variable is either a character vector or NULL
                  check_is_character <- sapply(variables, function(v) {
                      any(is.character(v), is.null(v))
                  })
                  check_is_character <- check_is_character[!check_is_character]
                  if(length(check_is_character)) {
                      stop(sprintf("\n%s", paste(names(check_is_character),
                                                 "must be character",
                                                 collapse="\n")))
                  }

                  .Call("set_config", repo, variables)
              }

              cfg <- .Call("get_config", repo)

              if(!length(variables)) {
                  lapply(names(cfg), function(level) {
                      cat(sprintf("%s:\n", level))
                      lapply(names(cfg[[level]]), function(entry) {
                          cat(sprintf("\t%s=%s\n",
                                      entry,
                                      cfg[[level]][[entry]][1]))
                      })
                  })
              }

              invisible(cfg)
          }
)
