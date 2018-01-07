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

##' Create tag targeting HEAD commit in repository
##'
##' @rdname tag-methods
##' @docType methods
##' @param object The repository \code{object}.
##' @param name Name for the tag.
##' @param message The tag message.
##' @param session Add sessionInfo to tag message. Default is FALSE.
##' @param tagger The tagger (author) of the tag
##' @return invisible(\code{git_tag}) object
##' @keywords methods
##' @include commit.r
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Commit a text file
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Create tag
##' tag(repo, "Tagname", "Tag message")
##'
##' ## List tags
##' tags(repo)
##' }
setGeneric("tag",
           signature = "object",
           function(object,
                    name,
                    message,
                    session = FALSE,
                    tagger  = default_signature(object))
           standardGeneric("tag"))

##' @rdname tag-methods
##' @export
setMethod("tag",
          signature(object = "git_repository"),
          function(object,
                   name,
                   message,
                   session,
                   tagger)
          {
              ## Argument checking
              stopifnot(is.character(name),
                        identical(length(name), 1L),
                        nchar(name[1]) > 0,
                        is.character(message),
                        identical(length(message), 1L),
                        nchar(message[1]) > 0,
                        is.logical(session),
                        identical(length(session), 1L),
                        is(tagger, "git_signature"))

              if (session)
                  message <- add_session_info(message)

              invisible(.Call(git2r_tag_create, object, name, message, tagger))
          }
)

##' Delete an existing tag reference
##'
##' @rdname tag_delete-methods
##' @docType methods
##' @param object Can be either a
##'     (\code{\linkS4class{git_repository}}) object, a
##'     \code{\linkS4class{git_tag}} object or the tag name. If the
##'     \code{object} argument is the tag name, the repository is
##'     searched for with \code{\link{discover_repository}} in the
##'     current working directory.
##' @param ... Additional arguments
##' @param name If the \code{object} argument is a
##'     \code{git_repository}, the name of the tag to delete.
##' @return \code{invisible(NULL)}
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Commit a text file
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Create two tags
##' tag(repo, "Tag1", "Tag message 1")
##' t2 <- tag(repo, "Tag2", "Tag message 2")
##'
##' ## List the two tags in the repository
##' tags(repo)
##'
##' ## Delete the two tags in the repository
##' tag_delete(repo, "Tag1")
##' tag_delete(t2)
##'
##' ## Show the empty list with tags in the repository
##' tags(repo)
##' }
setGeneric("tag_delete",
           signature = "object",
           function(object, ...)
           standardGeneric("tag_delete"))

##' @rdname tag_delete-methods
##' @export
setMethod("tag_delete",
          signature(object = "git_repository"),
          function(object, name)
          {
              invisible(.Call(git2r_tag_delete, object, name))
          }
)

##' @rdname tag_delete-methods
##' @export
setMethod("tag_delete",
          signature(object = "character"),
          function(object)
          {
              callGeneric(object = lookup_repository(), name = object)
          }
)

##' @rdname tag_delete-methods
##' @export
setMethod("tag_delete",
          signature(object = "git_tag"),
          function(object)
          {
              callGeneric(object = object@repo, name = object@name)
          }
)

##' Tags
##'
##' @template repo-param
##' @return list of tags in repository
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Commit a text file
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Create tag
##' tag(repo, "Tagname", "Tag message")
##'
##' ## List tags
##' tags(repo)
##' }
tags <- function(repo = NULL) {
    .Call(git2r_tag_list, lookup_repository(repo))
}

##' Brief summary of a tag
##'
##' @aliases show,git_tag-methods
##' @docType methods
##' @param object The tag \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Commit a text file
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Create tag
##' tag(repo, "Tagname", "Tag message")
##'
##' ## View brief summary of tag
##' tags(repo)[[1]]
##' }
setMethod("show",
          signature(object = "git_tag"),
          function(object)
          {
              cat(sprintf("[%s] %s\n",
                          substr(object@target, 1 , 6),
                          object@name))
          }
)

##' Summary of a tag
##'
##' @aliases summary,git_tag-methods
##' @docType methods
##' @param object The tag \code{object}
##' @param ... Additional arguments affecting the summary produced.
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Commit a text file
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Create tag
##' tag(repo, "Tagname", "Tag message")
##'
##' ## Summary of tag
##' summary(tags(repo)[[1]])
##' }
setMethod("summary",
          signature(object = "git_tag"),
          function(object, ...)
          {
              cat(sprintf(paste0("name:    %s\n",
                                 "target:  %s\n",
                                 "tagger:  %s <%s>\n",
                                 "when:    %s\n",
                                 "message: %s\n"),
                          object@name,
                          object@target,
                          object@tagger@name,
                          object@tagger@email,
                          as(object@tagger@when, "character"),
                          object@message))
          }
)
