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

##' Class \code{git_branch}
##'
##' @title S4 class to handle a git branch
##' @section Slots:
##' \describe{
##'   \item{name}{
##'     Name of the branch.
##'   }
##'   \item{type}{
##'     Type of the branch, either 1 (local) or 2 (remote).
##'   }
##'   \item{repo}{
##'     The S4 class git_repository that contains the branch
##'   }
##' }
##' @name git_branch-class
##' @docType class
##' @keywords classes
##' @section Methods:
##' \describe{
##'   \item{is_head}{\code{signature(object = "git_branch")}}
##'   \item{is_local}{\code{signature(object = "git_branch")}}
##'   \item{show}{\code{signature(object = "git_branch")}}
##' }
##' @keywords methods
##' @include reference.r
##' @include repository.r
##' @export
setClass("git_branch",
         slots = c(name = "character",
                   type = "integer",
                   repo = "git_repository"))

##' Create a branch
##'
##' @rdname branch_create-methods
##' @docType methods
##' @param commit Commit to which branch should point.
##' @param name Name for the branch
##' @param force Overwrite existing branch. Default = FALSE
##' @param message The one line long message to the reflog. Default is
##' NULL, which gives the log message "Branch: created"
##' @param who The identity that will be used to populate the
##' reflog entry. Default is NULL, which gives the default signature.
##' @return invisible S4 class git_branch object
##' @keywords methods
setGeneric("branch_create",
           signature = "commit",
           function(commit,
                    name,
                    force = FALSE,
                    message = NULL,
                    who = NULL)
           standardGeneric("branch_create"))

##' @rdname branch_create-methods
##' @export
setMethod("branch_create",
          signature = "git_commit",
          function(commit,
                   name,
                   force,
                   message,
                   who)
          {
              if (is.null(who)) {
                  who = default_signature(commit@repo)
              }

              invisible(.Call(git2r_branch_create, name, commit, force,
                              who, message))
          }
)

##' Delete a branch
##'
##' @rdname branch_delete-methods
##' @docType methods
##' @param branch The branch
##' @return invisible NULL
##' @keywords methods
setGeneric("branch_delete",
           signature = "branch",
           function(branch)
           standardGeneric("branch_delete"))

##' @rdname branch_delete-methods
##' @export
setMethod("branch_delete",
          signature = "git_branch",
          function(branch)
          {
              invisible(.Call(git2r_branch_delete, branch))
          }
)

##' Remote name of a branch
##'
##' The name of remote that the remote tracking branch belongs to
##' @rdname branch_remote_name-methods
##' @docType methods
##' @param branch The branch
##' @return character string with remote name
##' @keywords methods
setGeneric("branch_remote_name",
           signature = "branch",
           function(branch)
           standardGeneric("branch_remote_name"))

##' @rdname branch_remote_name-methods
##' @export
setMethod("branch_remote_name",
          signature = "git_branch",
          function(branch)
          {
              .Call(git2r_branch_remote_name, branch)
          }
)

##' Remote url of a branch
##'
##' @rdname branch_remote_url-methods
##' @docType methods
##' @param branch The branch
##' @return character string with remote url
##' @keywords methods
setGeneric("branch_remote_url",
           signature = "branch",
           function(branch)
           standardGeneric("branch_remote_url"))

##' @rdname branch_remote_url-methods
##' @export
setMethod("branch_remote_url",
          signature = "git_branch",
          function(branch)
          {
              .Call(git2r_branch_remote_url, branch)
          }
)

##' Rename a branch
##'
##' @rdname branch_rename-methods
##' @docType methods
##' @param branch Branch to rename
##' @param name The new name for the branch
##' @param force Overwrite existing branch. Default is FALSE
##' @param message The one line long message to the reflog. If NULL,
##' the default value is appended
##' @param who The identity that will be used to populate the
##' reflog entry. Default is NULL, which gives the default signature.
##' @return invisible renamed S4 class git_branch
##' @keywords methods
setGeneric("branch_rename",
           signature = "branch",
           function(branch,
                    name,
                    force = FALSE,
                    message = NULL,
                    who = NULL)
           standardGeneric("branch_rename"))

##' @rdname branch_rename-methods
##' @export
setMethod("branch_rename",
          signature = "git_branch",
          function(branch,
                   name,
                   force,
                   message,
                   who)
          {
              if (is.null(who)) {
                  who = default_signature(branch@repo)
              }

              invisible(.Call(git2r_branch_rename, branch, name, force,
                              who, message))
          }
)

##' Get target (hex) pointed to by a branch
##'
##' @rdname branch_target-methods
##' @docType methods
##' @param branch The branch
##' @return hex or NA if not a direct reference
##' @keywords methods
setGeneric("branch_target",
           signature = "branch",
           function(branch)
           standardGeneric("branch_target"))

##' @rdname branch_target-methods
##' @export
setMethod("branch_target",
          signature = "git_branch",
          function(branch)
          {
              .Call(git2r_branch_target, branch)
          }
)

##' Get remote tracking branch
##'
##' Get remote tracking branch, given a local branch.
##' @rdname branch_get_upstream-methods
##' @docType methods
##' @param branch The branch
##' @return S4 class git_branch or NULL if no remote tracking branch.
##' @keywords methods
setGeneric("branch_get_upstream",
           signature = "branch",
           function(branch)
           standardGeneric("branch_get_upstream"))

##' @rdname branch_get_upstream-methods
##' @export
setMethod("branch_get_upstream",
          signature = "git_branch",
          function(branch)
          {
              .Call(git2r_branch_get_upstream, branch)
          }
)

##' Set remote tracking branch
##'
##' Set the upstream configuration for a given local branch
##' @rdname branch_set_upstream-methods
##' @docType methods
##' @param branch The branch to configure
##' @param name remote-tracking or local branch to set as
##' upstream. Pass NULL to unset.
##' @return invisible NULL
##' @keywords methods
setGeneric("branch_set_upstream",
           signature = "branch",
           function(branch, name)
           standardGeneric("branch_set_upstream"))

##' @rdname branch_set_upstream-methods
##' @export
setMethod("branch_set_upstream",
          signature(branch = "git_branch"),
          function(branch, name)
          {
              if (missing(name)) {
                  stop("Missing argument name")
              }
              invisible(.Call(git2r_branch_set_upstream, branch, name))
          }
)

##' Branches
##'
##' List branches in repository
##' @rdname branches-methods
##' @docType methods
##' @param repo The repository \code{object}
##' @param flags Filtering flags for the branch listing. Valid values
##' are 'all', 'local' or 'remote'
##' @return list of branches in repository
##' @keywords methods
setGeneric("branches",
           signature = "repo",
           function(repo, flags=c("all", "local", "remote"))
           standardGeneric("branches"))

##' @rdname branches-methods
##' @export
setMethod("branches",
          signature(repo = "git_repository"),
          function (repo, flags)
          {
              flags <- switch(match.arg(flags),
                              local  = 1L,
                              remote = 2L,
                              all    = 3L)

              .Call(git2r_branch_list, repo, flags)
          }
)

##' Check if branch is head
##'
##' @rdname is_head-methods
##' @docType methods
##' @param branch The branch \code{object} to check if it's head
##' @return TRUE if branch is head, else FALSE
##' @keywords methods
setGeneric("is_head",
           signature = "branch",
           function(branch)
           standardGeneric("is_head"))

##' @rdname is_head-methods
##' @export
setMethod("is_head",
          signature(branch = "git_branch"),
          function (branch)
          {
              .Call(git2r_branch_is_head, branch)
          }
)

##' Check if branch is local
##'
##' @rdname is_local-methods
##' @docType methods
##' @param branch The branch \code{object} to check if it's local
##' @return TRUE if branch is local, else FALSE
##' @keywords methods
setGeneric("is_local",
           signature = "branch",
           function(branch)
           standardGeneric("is_local"))

##' @rdname is_local-methods
##' @export
setMethod("is_local",
          signature(branch = "git_branch"),
          function (branch)
          {
              identical(branch@type, 1L)
          }
)

##' Brief summary of branch
##'
##' @aliases show,git_branch-methods
##' @docType methods
##' @param object The branch \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
setMethod("show",
          signature(object = "git_branch"),
          function (object)
          {
              hex <- branch_target(object)
              if (!is.na(hex)) {
                  cat(sprintf("[%s] ", substr(hex, 1 , 6)))
              }

              if (is_local(object)) {
                  cat("(Local) ")
              } else {
                  cat(sprintf("(%s @ %s) ",
                              branch_remote_name(object),
                              branch_remote_url(object)))
              }

              if (is_head(object)) {
                  cat("(HEAD) ")
              }

              if (is_local(object)) {
                  cat(sprintf("%s\n", object@name))
              } else {
                  cat(sprintf("%s\n",
                              substr(object@name,
                                     start = nchar(branch_remote_name(object)) + 2,
                                     stop = nchar(object@name))))
              }
          }
)
