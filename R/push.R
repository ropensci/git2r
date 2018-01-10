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

##' Push
##'
##' @rdname push-methods
##' @docType methods
##' @param object S4 class \code{git_repository} or \code{git_branch}.
##' @param ... Additional arguments affecting the push.
##' @param force Force your local revision to the remote repo. Use it
##' with care. Default is FALSE.
##' @param credentials The credentials for remote repository
##' access. Default is NULL. To use and query an ssh-agent for the ssh
##' key credentials, let this parameter be NULL (the default).
##' @return invisible(NULL)
##' @seealso \code{\linkS4class{cred_user_pass}},
##' \code{\linkS4class{cred_ssh_key}}
##' @keywords methods
##' @include refspec.R
##' @include S4_classes.R
##' @examples
##' \dontrun{
##' ## Initialize two temporary repositories
##' path_bare <- tempfile(pattern="git2r-")
##' path_repo <- tempfile(pattern="git2r-")
##' dir.create(path_bare)
##' dir.create(path_repo)
##' repo_bare <- init(path_bare, bare = TRUE)
##' repo <- clone(path_bare, path_repo)
##'
##' ## Config user and commit a file
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Write to a file and commit
##' writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'            file.path(path_repo, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Push commits from repository to bare repository
##' ## Adds an upstream tracking branch to branch 'master'
##' push(repo, "origin", "refs/heads/master")
##'
##' ## Change file and commit
##' writeLines(c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'              "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
##'            file.path(path_repo, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "Second commit message")
##'
##' ## Push commits from repository to bare repository
##' push(repo)
##'
##' ## List commits in repository and bare repository
##' commits(repo)
##' commits(repo_bare)
##' }
setGeneric("push",
           signature = "object",
           function(object, ...)
           standardGeneric("push"))

##' @rdname push-methods
##' @export
setMethod("push",
          signature(object = "git_branch"),
          function(object,
                   force       = FALSE,
                   credentials = NULL)
          {
              upstream <- branch_get_upstream(object)
              if (is.null(upstream)) {
                  stop("The branch '", object@name, "' that you are ",
                       "trying to push does not track an upstream branch.")
              }

              src <- .Call(git2r_branch_canonical_name, object)
              dst <- .Call(git2r_branch_upstream_canonical_name, object)

              push(object      = object@repo,
                   name        = branch_remote_name(upstream),
                   refspec     = paste0(src, ":", dst),
                   force       = force,
                   credentials = credentials)
          }
)

##' @rdname push-methods
##' @param name The remote's name. Default is NULL.
##' @param refspec The refspec to be pushed. Default is NULL.
##' @export
setMethod("push",
          signature(object = "git_repository"),
          function(object,
                   name        = NULL,
                   refspec     = NULL,
                   force       = FALSE,
                   credentials = NULL)
          {
              if (all(is.null(name), is.null(refspec))) {
                  b <- head(object)
                  upstream <- branch_get_upstream(b)
                  if (is.null(upstream)) {
                      stop("The branch '", b@name, "' that you are ",
                           "trying to push does not track an upstream branch.")
                  }

                  src <- .Call(git2r_branch_canonical_name, b)
                  dst <- .Call(git2r_branch_upstream_canonical_name, b)
                  name <- branch_remote_name(upstream)
                  refspec <- paste0(src, ":", dst)

                  if (isTRUE(force))
                      refspec <- paste0("+", refspec)
              } else {
                  opts <- list(force = force)
                  tmp <- get_refspec(object, name, refspec, opts)
                  name <- tmp$remote
                  refspec <- tmp$refspec
              }

              result <- .Call(git2r_push, object, name, refspec, credentials)

              invisible(result)
          }
)
