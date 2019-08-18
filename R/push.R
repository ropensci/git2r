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

get_upstream_name <- function(object) {
    upstream <- branch_get_upstream(object)

    if (is.null(upstream)) {
        stop("The branch '", object$name, "' that you are ",
             "trying to push does not track an upstream branch.")
    }

    branch_remote_name(upstream)
}

##' Push
##'
##' @param object path to repository, or a \code{git_repository} or
##'     \code{git_branch}.
##' @param name The remote's name. Default is NULL.
##' @param refspec The refspec to be pushed. Default is NULL.
##' @param force Force your local revision to the remote repo. Use it
##'     with care. Default is FALSE.
##' @param credentials The credentials for remote repository
##'     access. Default is NULL. To use and query an ssh-agent for the
##'     ssh key credentials, let this parameter be NULL (the default).
##' @param set_upstream Set the current local branch to track the
##'     remote branch. Default is FALSE.
##' @return invisible(NULL)
##' @seealso \code{\link{cred_user_pass}}, \code{\link{cred_ssh_key}}
##' @export
##' @examples
##' \dontrun{
##' ## Initialize two temporary repositories
##' path_bare <- tempfile(pattern="git2r-")
##' path_repo <- tempfile(pattern="git2r-")
##' dir.create(path_bare)
##' dir.create(path_repo)
##' repo_bare <- init(path_bare, bare = TRUE)
##'
##' ## Clone the bare repository. This creates remote-tracking
##' ## branches for each branch in the cloned repository.
##' repo <- clone(path_bare, path_repo)
##'
##' ## Config user and commit a file
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Write to a file and commit
##' lines <- "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do"
##' writeLines(lines, file.path(path_repo, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Push commits from repository to bare repository
##' push(repo, "origin", "refs/heads/master")
##'
##' ## Now, unset the remote-tracking branch to NULL to demonstrate
##' ## the 'set_upstream' argument. Then push with 'set_upstream = TRUE'
##' ## to add the upstream tracking branch to branch 'master' again.
##' branch_get_upstream(repository_head(repo))
##' branch_set_upstream(repository_head(repo), NULL)
##' branch_get_upstream(repository_head(repo))
##' push(repo, "origin", "refs/heads/master", set_upstream = TRUE)
##' branch_get_upstream(repository_head(repo))
##'
##' ## Change file and commit
##' lines <- c(
##'   "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'   "eiusmod tempor incididunt ut labore et dolore magna aliqua.")
##' writeLines(lines, file.path(path_repo, "example.txt"))
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
push <- function(object       = ".",
                 name         = NULL,
                 refspec      = NULL,
                 force        = FALSE,
                 credentials  = NULL,
                 set_upstream = FALSE) {
    if (is_branch(object)) {
        name <- get_upstream_name(object)

        src <- .Call(git2r_branch_canonical_name, object)
        dst <- .Call(git2r_branch_upstream_canonical_name, object)
        refspec <- paste0(src, ":", dst)
        object <- object$repo
    } else {
        object <- lookup_repository(object)
    }

    if (all(is.null(name), is.null(refspec))) {
        b <- repository_head(object)
        name <- get_upstream_name(b)
        src <- .Call(git2r_branch_canonical_name, b)
        dst <- .Call(git2r_branch_upstream_canonical_name, b)
        refspec <- paste0(src, ":", dst)

        if (isTRUE(force))
            refspec <- paste0("+", refspec)
    } else {
        opts <- list(force = force)
        tmp <- get_refspec(object, name, refspec, opts)
        name <- tmp$remote
        refspec <- tmp$refspec
    }

    .Call(git2r_push, object, name, refspec, credentials)

    if (isTRUE(set_upstream)) {
        b <- repository_head(object)
        if (is_local(b))
            branch_set_upstream(b, paste0(name, "/", b$name))
    }

    invisible(NULL)
}
