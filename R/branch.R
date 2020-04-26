## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2020 The git2r contributors
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

##' Create a branch
##'
##' @param commit Commit to which the branch should point. The default
##'     is to use the \code{last_commit()} function to determine the
##'     commit to which the branch should point.
##' @param name Name for the branch
##' @param force Overwrite existing branch. Default = FALSE
##' @return invisible git_branch object
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user and commit a file
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##' lines <- "Hello world!"
##' writeLines(lines, file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit_1 <- commit(repo, "First commit message")
##'
##' ## Create a branch
##' branch_1 <- branch_create(commit_1, name = "test-branch")
##'
##' ## Add one more commit
##' lines <- c("Hello world!", "HELLO WORLD!")
##' writeLines(lines, file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit_2 <- commit(repo, "Another commit message")
##'
##' ## Create a branch with the same name should fail
##' try(branch_create(commit_2, name = "test-branch"), TRUE)
##'
##' ## Force it
##' branch_2 <- branch_create(commit_2, name = "test-branch", force = TRUE)
##' }
branch_create <- function(commit = last_commit(), name = NULL,
                          force = FALSE) {
    invisible(.Call(git2r_branch_create, name, commit, force))
}

##' Delete a branch
##'
##' @param branch The branch
##' @return invisible NULL
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user and commit a file
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit_1 <- commit(repo, "First commit message")
##'
##' ## Create a 'dev' branch
##' dev <- branch_create(commit_1, name = "dev")
##' branches(repo)
##'
##' ## Delete 'dev' branch
##' branch_delete(dev)
##' branches(repo)
##' }
branch_delete <- function(branch = NULL) {
    .Call(git2r_branch_delete, branch)
    invisible(NULL)
}

##' Remote name of a branch
##'
##' The name of remote that the remote tracking branch belongs to
##' @param branch The branch
##' @return character string with remote name
##' @export
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
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Write to a file and commit
##' lines <- "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do"
##' writeLines(lines, file.path(path_repo, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Push commits from repository to bare repository
##' ## Adds an upstream tracking branch to branch 'master'
##' push(repo, "origin", "refs/heads/master")
##'
##' ## Get remote name
##' branch_remote_name(branches(repo)[[2]])
##' }
branch_remote_name <- function(branch = NULL) {
    .Call(git2r_branch_remote_name, branch)
}

##' Remote url of a branch
##'
##' @param branch The branch
##' @return character string with remote url
##' @export
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
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Write to a file and commit
##' lines <- "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do"
##' writeLines(lines, file.path(path_repo, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Push commits from repository to bare repository
##' ## Adds an upstream tracking branch to branch 'master'
##' push(repo, "origin", "refs/heads/master")
##'
##' ## Get remote url of tracking branch to branch 'master'
##' branch_remote_url(branch_get_upstream(repository_head(repo)))
##' }
branch_remote_url <- function(branch = NULL) {
    .Call(git2r_branch_remote_url, branch)
}

##' Rename a branch
##'
##' @param branch Branch to rename
##' @param name The new name for the branch
##' @param force Overwrite existing branch. Default is FALSE
##' @return invisible renamed \code{git_branch} object
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Config user and commit a file
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##' lines <- "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do"
##' writeLines(lines, file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Rename 'master' branch to 'dev'
##' branches(repo)
##' branch_rename(repository_head(repo), "dev")
##' branches(repo)
##' }
branch_rename <- function(branch = NULL, name = NULL, force = FALSE) {
    invisible(.Call(git2r_branch_rename, branch, name, force))
}

##' Get target (sha) pointed to by a branch
##'
##' @param branch The branch
##' @return sha or NA if not a direct reference
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Config user and commit a file
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##' lines <- "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do"
##' writeLines(lines, file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Get target (sha) pointed to by 'master' branch
##' branch_target(repository_head(repo))
##' }
branch_target <- function(branch = NULL) {
    .Call(git2r_branch_target, branch)
}

##' Get remote tracking branch
##'
##' Get remote tracking branch, given a local branch.
##' @param branch The branch
##' @return \code{git_branch} object or NULL if no remote tracking
##'     branch.
##' @export
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
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Write to a file and commit
##' lines <- "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do"
##' writeLines(lines, file.path(path_repo, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Push commits from repository to bare repository
##' ## Adds an upstream tracking branch to branch 'master'
##' push(repo, "origin", "refs/heads/master")
##'
##' ## Get remote tracking branch
##' branch_get_upstream(repository_head(repo))
##' }
branch_get_upstream <- function(branch = NULL) {
    .Call(git2r_branch_get_upstream, branch)
}

##' Set remote tracking branch
##'
##' Set the upstream configuration for a given local branch
##' @param branch The branch to configure
##' @param name remote-tracking or local branch to set as
##' upstream. Pass NULL to unset.
##' @return invisible NULL
##' @export
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
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Write to a file and commit
##' lines <- "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do"
##' writeLines(lines, file.path(path_repo, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Push commits from repository to bare repository
##' ## Adds an upstream tracking branch to branch 'master'
##' push(repo, "origin", "refs/heads/master")
##'
##' ## Unset remote remote tracking branch
##' branch_get_upstream(repository_head(repo))
##' branch_set_upstream(repository_head(repo), NULL)
##' branch_get_upstream(repository_head(repo))
##'
##' ## Set remote tracking branch
##' branch_set_upstream(repository_head(repo), "origin/master")
##' branch_get_upstream(repository_head(repo))
##' }
branch_set_upstream <- function(branch = NULL, name) {
    if (missing(name))
        stop("Missing argument name")
    .Call(git2r_branch_set_upstream, branch, name)
    invisible(NULL)
}

##' Branches
##'
##' List branches in repository
##' @template repo-param
##' @param flags Filtering flags for the branch listing. Valid values
##'     are 'all', 'local' or 'remote'
##' @return list of branches in repository
##' @export
##' @examples
##' \dontrun{
##' ## Initialize repositories
##' path_bare <- tempfile(pattern="git2r-")
##' path_repo <- tempfile(pattern="git2r-")
##' dir.create(path_bare)
##' dir.create(path_repo)
##' repo_bare <- init(path_bare, bare = TRUE)
##' repo <- clone(path_bare, path_repo)
##'
##' ## Config first user and commit a file
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Write to a file and commit
##' lines <- "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do"
##' writeLines(lines, file.path(path_repo, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Push commits from repository to bare repository
##' ## Adds an upstream tracking branch to branch 'master'
##' push(repo, "origin", "refs/heads/master")
##'
##' ## List branches
##' branches(repo)
##' }
branches <- function(repo = ".", flags=c("all", "local", "remote")) {
    flags <- switch(match.arg(flags),
                    local  = 1L,
                    remote = 2L,
                    all    = 3L)

    .Call(git2r_branch_list, lookup_repository(repo), flags)
}

##' Check if branch is head
##'
##' @param branch The branch \code{object} to check if it's head.
##' @return \code{TRUE} if branch is head, else \code{FALSE}.
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user and commit a file
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## List branches
##' branches(repo)
##'
##' ## Check that 'master' is_head
##' master <- branches(repo)[[1]]
##' is_head(master)
##'
##' ## Create and checkout 'dev' branch
##' checkout(repo, "dev", create = TRUE)
##'
##' ## List branches
##' branches(repo)
##'
##' ## Check that 'master' is no longer head
##' is_head(master)
##' }
is_head <- function(branch = NULL) {
    .Call(git2r_branch_is_head, branch)
}

##' Check if branch is local
##'
##' @param branch The branch \code{object} to check if it's local
##' @return \code{TRUE} if branch is local, else \code{FALSE}.
##' @export
##' @examples
##' \dontrun{
##' ## Initialize repositories
##' path_bare <- tempfile(pattern="git2r-")
##' path_repo <- tempfile(pattern="git2r-")
##' dir.create(path_bare)
##' dir.create(path_repo)
##' repo_bare <- init(path_bare, bare = TRUE)
##' repo <- clone(path_bare, path_repo)
##'
##' ## Config first user and commit a file
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Write to a file and commit
##' lines <- "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do"
##' writeLines(lines, file.path(path_repo, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Push commits from repository to bare repository
##' ## Adds an upstream tracking branch to branch 'master'
##' push(repo, "origin", "refs/heads/master")
##'
##' ## List branches
##' branches(repo)
##'
##' ## Check if first branch is_local
##' is_local(branches(repo)[[1]])
##'
##' ## Check if second branch is_local
##' is_local(branches(repo)[[2]])
##' }
is_local <- function(branch) {
    if (!is_branch(branch))
        stop("argument 'branch' must be a 'git_branch' object")
    identical(branch$type, 1L)
}

##' @export
print.git_branch <- function(x, ...) {
    sha <- branch_target(x)
    if (!is.na(sha)) {
        cat(sprintf("[%s] ", substr(sha, 1, 6)))
    }

    if (is_local(x)) {
        cat("(Local) ")
    } else {
        cat(sprintf("(%s @ %s) ",
                    branch_remote_name(x),
                    branch_remote_url(x)))
    }

    if (is_head(x)) {
        cat("(HEAD) ")
    }

    if (is_local(x)) {
        cat(sprintf("%s\n", x$name))
    } else {
        cat(sprintf("%s\n",
                    substr(x$name,
                           start = nchar(branch_remote_name(x)) + 2,
                           stop = nchar(x$name))))
    }

    invisible(x)
}

##' Check if object is \code{git_branch}
##'
##' @param object Check if object is of class \code{git_branch}
##' @return TRUE if object is class \code{git_branch}, else FALSE
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Commit a text file
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' branch <- branches(repo)[[1]]
##'
##' ## Check if branch
##' is_branch(branch)
##' }
is_branch <- function(object) {
    inherits(object, "git_branch")
}
