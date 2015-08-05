## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2015 The git2r contributors
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
##'
##' @rdname references-methods
##' @docType methods
##' @param repo The repository \code{object}
##' \code{\linkS4class{git_repository}}. If the \code{repo} argument
##' is missing, the repository is searched for with
##' \code{\link{discover_repository}} in the current working
##' directory.
##' @return Character vector with references
##' @keywords methods
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
setGeneric("references",
           signature = "repo",
           function(repo) standardGeneric("references"))

##' @rdname references-methods
##' @include S4_classes.r
##' @export
setMethod("references",
          signature(repo = "missing"),
          function()
          {
              callGeneric(repo = lookup_repository())
          }
)

##' @rdname references-methods
##' @include S4_classes.r
##' @export
setMethod("references",
          signature(repo = "git_repository"),
          function(repo)
          {
              .Call(git2r_reference_list, repo)
          }
)

##' Brief summary of reference
##'
##' @aliases show,git_reference-methods
##' @docType methods
##' @param object The reference \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @include S4_classes.r
##' @export
##' @examples
##' \dontrun{
##' ## Create and initialize a repository in a temporary directory
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Create a file, add and commit
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Brief summary of reference
##' references(repo)[[1]]
##' }
setMethod("show",
          signature(object = "git_reference"),
          function(object)
          {
              if (identical(object@type, 1L)) {
                  cat(sprintf("[%s] %s\n",
                              substr(object@sha, 1 , 6),
                              object@shorthand))
              } else if (identical(object@type, 2L)) {
                  cat(sprintf("%s => %s\n",
                              object@name,
                              object@target))
              } else {
                  stop("Unexpected reference type")
              }
          }
)
