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

##' Time
##'
##' The class \code{git_time} stores the time a Git object was created.
##'
##' The default is to use \code{origin = "1970-01-01"} and \code{tz = "GMT"}. To
##' use your local timezone, set \code{tz = Sys.timezone()}.
##'
##' @inheritParams base::as.POSIXct
##' @inheritParams base::strptime
##' @seealso \code{\link{when}}
##' @name git_time
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
##' as.POSIXct(commits(repo)[[1]]$author$when)
##' as.POSIXct(tags(repo)[[1]]$tagger$when)
##' as.POSIXct(tags(repo)[[1]]$tagger$when, tz = Sys.timezone())
##' }
NULL

##' @rdname git_time
##' @export
as.character.git_time <- function(x,  origin = "1970-01-01", tz = "GMT", usetz = TRUE, ...) {
    as.character(format(as.POSIXct(x, origin = origin, tz = tz), usetz = usetz), ...)
}

##' @rdname git_time
##' @export
format.git_time <- function(x,  origin = "1970-01-01", tz = "GMT", usetz = TRUE, ...) {
    format(as.POSIXct(x, origin = origin, tz = tz), usetz = usetz, ...)
}

##' @rdname git_time
##' @export
as.POSIXct.git_time <- function(x, origin = "1970-01-01", tz = "GMT", ...) {
    as.POSIXct(x$time, origin = origin, tz = tz, ...)
}

##' @rdname git_time
##' @export
print.git_time <- function(x,  origin = "1970-01-01", tz = "GMT", usetz = TRUE, ...) {
    cat(sprintf("%s\n", as.character(x, origin = origin, tz = tz, usetz = usetz, ...)))
    invisible(x)
}
