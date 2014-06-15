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
##' @title S4_class_to_handle_a_git_branch
##' @section Slots:
##' \describe{
##'   \item{remote}{
##'     The name of remote that the remote tracking branch belongs to
##'   }
##'   \item{url}{
##'     The remote's url
##'   }
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
         slots = c(remote = "character",
                   url    = "character",
                   name   = "character",
                   type   = "integer",
                   repo   = "git_repository"),
         prototype = list(remote = NA_character_,
                          url    = NA_character_))

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
              .Call("git2r_branch_remote_name", branch)
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
              .Call("git2r_branch_target", branch)
          }
)

##' Branches
##'
##' List branches in repository
##' @rdname branches-methods
##' @docType methods
##' @param object The repository
##' @param flags Filtering flags for the branch listing. Valid values
##' are 'ALL', 'LOCAL' or 'REMOTE'
##' @return list of branches in repository
##' @keywords methods
setGeneric("branches",
           signature = "object",
           function(object, flags=c("ALL", "LOCAL", "REMOTE"))
           standardGeneric("branches"))

##' @rdname branches-methods
##' @export
setMethod("branches",
          signature(object = "git_repository"),
          function (object, flags)
          {
              flags <- switch(match.arg(flags),
                              LOCAL  = 1L,
                              REMOTE = 2L,
                              ALL    = 3L)

              .Call("git2r_branch_list", object, flags)
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
              .Call("git2r_branch_is_head", branch)
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
              if(identical(object@type, 1L)) {
                  cat(sprintf("[%s] ", substr(object@hex, 1 , 6)))
              }

              if(is_local(object)) {
                  cat("(Local) ")
              } else {
                  cat(sprintf("(%s @ %s) ", object@remote, object@url))
              }

              if(is_head(object)) {
                  cat("(HEAD) ")
              }

              if(identical(object@type, 1L)) {
                  if(is_local(object)) {
                      cat(sprintf("%s\n", object@shorthand))
                  } else {
                      cat(sprintf("%s\n",
                                  substr(object@shorthand,
                                         start=nchar(object@remote)+2,
                                         stop=nchar(object@shorthand))))
                  }
              } else if(identical(object@type, 2L)) {
                  cat(sprintf("%s => %s\n", object@name, object@target))
              } else {
                  stop("Unexpected reference type")
              }
          }
)
