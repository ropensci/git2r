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

##' When
##'
##' Help method to extract the time as a character string from a
##' git_commit, git_signature, git_tag and git_time object.
##' @param object the \code{object} to extract the time slot from.
##' @inheritParams base::as.POSIXct
##' @inheritParams base::strptime
##' @return A \code{character} vector of length one.
##' @seealso \code{\link{git_time}}
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a first user and commit a file
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Create tag
##' tag(repo, "Tagname", "Tag message")
##'
##' when(commits(repo)[[1]])
##' when(tags(repo)[[1]])
##' when(tags(repo)[[1]], tz = Sys.timezone())
##' }
when <- function(object, tz = "GMT", origin = "1970-01-01", usetz = TRUE) {
    if (inherits(object, "git_commit"))
        return(as.character(object$author$when, tz = tz, origin = origin,
                            usetz = usetz))
    if (inherits(object, "git_signature"))
        return(as.character(object$when, tz = tz, origin = origin,
                            usetz = usetz))
    if (inherits(object, "git_stash"))
        return(as.character(object$stasher$when, tz = tz, origin = origin,
                            usetz = usetz))
    if (inherits(object, "git_tag"))
        return(as.character(object$tagger$when, tz = tz, origin = origin,
                            usetz = usetz))
    if (inherits(object, "git_time"))
        return(as.character(object, tz = tz, origin = origin,
                            usetz = usetz))
    stop("Invalid 'object'")
}
