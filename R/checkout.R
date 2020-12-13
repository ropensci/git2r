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

##' Determine previous branch name
##'
##' @param repo The repository.
##' @noRd
previous_branch_name <- function(repo) {
    branch <- revparse_single(repo, "@{-1}")$sha
    branch <- sapply(references(repo), function(x) {
        if (is.null(x$sha))
            return(NA_character_)
        if (x$sha == branch)
            return(x$shorthand)
        NA_character_
    })
    branch <- branch[vapply(branch, Negate(is.na), logical(1))]

    branch <- sapply(branches(repo, "local"), function(x) {
        ifelse(x$name %in% branch, x$name, NA_character_)
    })
    branch <- branch[vapply(branch, Negate(is.na), logical(1))]

    if (any(!is.character(branch), !identical(length(branch), 1L))) {
        stop("'branch' must be a character vector of length one")
    }

    branch
}

checkout_branch <- function(object, force) {
    ref_name <- paste0("refs/heads/", object$name)
    .Call(git2r_checkout_tree, object$repo, ref_name, force)
    .Call(git2r_repository_set_head, object$repo, ref_name)
}

checkout_commit <- function(object, force) {
    .Call(git2r_checkout_tree, object$repo, object$sha, force)
    .Call(git2r_repository_set_head_detached, object)
}

checkout_tag <- function(object, force) {
    .Call(git2r_checkout_tree, object$repo, object$target, force)
    .Call(git2r_repository_set_head_detached,
          lookup(object$repo, object$target))
}

checkout_git_object <- function(object, force) {
    if (is_branch(object)) {
        checkout_branch(object, force)
        return(TRUE)
    }

    if (is_commit(object)) {
        checkout_commit(object, force)
        return(TRUE)
    }

    if (is_tag(object)) {
        checkout_tag(object, force)
        return(TRUE)
    }

    FALSE
}

##' Checkout
##'
##' Update files in the index and working tree to match the content of
##' the tree pointed at by the treeish object (commit, tag or tree).
##' The default checkout strategy (\code{force = FALSE}) will only
##' make modifications that will not lose changes. Use \code{force =
##' TRUE} to force working directory to look like index.
##' @param object A path to a repository, or a \code{git_repository}
##'     object, or a \code{git_commit} object, or a \code{git_tag}
##'     object, or a \code{git_tree} object.
##' @param branch name of the branch to check out. Only used if object
##'     is a path to a repository or a \code{git_repository} object.
##' @param create create branch if it doesn't exist. Only used if
##'     object is a path to a repository or a \code{git_repository}
##'     object.
##' @param force If \code{TRUE}, then make working directory match
##'     target. This will throw away local changes. Default is
##'     \code{FALSE}.
##' @param path Limit the checkout operation to only certain
##'     paths. This argument is only used if branch is NULL. Default
##'     is \code{NULL}.
##' @param ...  Additional arguments. Not used.
##' @return invisible NULL
##' @export
##' @examples
##' \dontrun{
##' ## Create directories and initialize repositories
##' path_bare <- tempfile(pattern="git2r-")
##' path_repo_1 <- tempfile(pattern="git2r-")
##' path_repo_2 <- tempfile(pattern="git2r-")
##' dir.create(path_bare)
##' dir.create(path_repo_1)
##' dir.create(path_repo_2)
##' repo_bare <- init(path_bare, bare = TRUE)
##'
##' ## Clone to repo 1 and config user
##' repo_1 <- clone(path_bare, path_repo_1)
##' config(repo_1, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Add changes to repo 1 and push to bare
##' lines <- "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do"
##' writeLines(lines, file.path(path_repo_1, "test.txt"))
##' add(repo_1, "test.txt")
##' commit(repo_1, "First commit message")
##' push(repo_1, "origin", "refs/heads/master")
##'
##' ## Create and checkout 'dev' branch in repo 1
##' checkout(repo_1, "dev", create = TRUE)
##'
##' ## Add changes to 'dev' branch in repo 1 and push to bare
##' lines <- c(
##'   "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'   "eiusmod tempor incididunt ut labore et dolore magna aliqua.")
##' writeLines(lines, file.path(path_repo_1, "test.txt"))
##' add(repo_1, "test.txt")
##' commit(repo_1, "Second commit message")
##' push(repo_1, "origin", "refs/heads/dev")
##'
##' ## Clone to repo 2
##' repo_2 <- clone(path_bare, path_repo_2)
##' config(repo_2, user.name = "Bob", user.email = "bob@@example.org")
##'
##' ## Read content of 'test.txt'
##' readLines(file.path(path_repo_2, "test.txt"))
##'
##' ## Checkout dev branch
##' checkout(repo_2, "dev")
##'
##' ## Read content of 'test.txt'
##' readLines(file.path(path_repo_2, "test.txt"))
##'
##' ## Edit "test.txt" in repo_2
##' writeLines("Hello world!", con = file.path(path_repo_2, "test.txt"))
##'
##' ## Check status
##' status(repo_2)
##'
##' ## Checkout "test.txt"
##' checkout(repo_2, path = "test.txt")
##'
##' ## Check status
##' status(repo_2)
##' }
checkout <- function(object = NULL,
                     branch = NULL,
                     create = FALSE,
                     force  = FALSE,
                     path   = NULL,
                     ...) {
    if (checkout_git_object(object, force))
        return(invisible(NULL))

    object <- lookup_repository(object)
    if (is.null(branch)) {
        if (is.null(path))
            stop("missing 'branch' or 'path' argument")
        .Call(git2r_checkout_path, object, path)
        return(invisible(NULL))
    }

    if (!is.character(branch) || !identical(length(branch), 1L))
        stop("'branch' must be a character vector of length one")

    if (is_empty(object)) {
        if (!isTRUE(create))
            stop(sprintf("'%s' did not match any branch", branch))
        ref_name <- paste0("refs/heads/", branch)
        .Call(git2r_repository_set_head, object, ref_name)
        return(invisible(NULL))
    }

    if (identical(branch, "-"))
        branch <- previous_branch_name(object)

    ## Check if branch exists in a local branch
    lb <- branches(object, "local")
    lb <- lb[vapply(lb, "[[", character(1), "name") == branch]
    if (length(lb)) {
        checkout_branch(lb[[1]], force)
        return(invisible(NULL))
    }

    ## Check if there exists exactly one remote branch with a matching
    ## name.
    rb <- branches(object, "remote")

    ## Split remote/name to check for a unique name
    name <- vapply(rb, function(x) {
        remote <- strsplit(x$name, "/")[[1]][1]
        sub(paste0("^", remote, "/"), "", x$name)
    }, character(1))
    i <- which(name == branch)
    if (identical(length(i), 1L)) {
        ## Create branch and track remote
        commit <- lookup(object, branch_target(rb[[i]]))
        branch <- branch_create(commit, branch)
        branch_set_upstream(branch, rb[[i]]$name)
        checkout_branch(branch, force)
        return(invisible(NULL))
    }

    if (isTRUE(create)) {
        ## Create branch
        commit <- lookup(object, branch_target(repository_head(object)))
        checkout_branch(branch_create(commit, branch), force)
        return(invisible(NULL))
    }

    ## Check if branch object is specified by revision.
    if (checkout_git_object(revparse_single(object, branch), force))
        return(invisible(NULL))

    stop(sprintf("'%s' did not match any branch", branch))
}
