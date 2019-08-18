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

##' Get all references that can be found in a repository.
##' @template repo-param
##' @return Character vector with references
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
##' ## Add tag to HEAD
##' tag(repo, "v1.0", "First version")
##'
##' ## Create a note
##' note_create(commits(repo)[[1]], "My note")
##'
##' ## List all references in repository
##' references(repo)
##' }
##'
references <- function(repo = ".") {
    .Call(git2r_reference_list, lookup_repository(repo))
}

##' @export
format.git_reference <- function(x, ...) {
    if (identical(x$type, 1L))
        return(sprintf("[%s] %s", substr(x$sha, 1, 6), x$shorthand))
    sprintf("%s => %s", x$name, x$target)
}

##' @export
print.git_reference <- function(x, ...) {
    cat(format(x, ...), "\n", sep = "")
    invisible(x)
}
