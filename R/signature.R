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

##' Get the signature
##'
##' Get the signature according to the repository's configuration
##' @template repo-param
##' @return A \code{git_signature} object with entries:
## \describe{
##   \item{name}{
##     The full name of the author.
##   }
##   \item{email}{
##     Email of the author.
##   }
##   \item{when}{
##     Time when the action happened.
##   }
## }
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
##' ## Get the default signature
##' default_signature(repo)
##'
##' ## Change user
##' config(repo, user.name = "Bob", user.email = "bob@@example.org")
##'
##' ## Get the default signature
##' default_signature(repo)
##' }
default_signature <- function(repo = ".") {
    .Call(git2r_signature_default, lookup_repository(repo))
}

##' @export
format.git_signature <- function(x, ...) {
    sprintf("name:  %s\nemail: %s\nwhen:  %s",
            x$name, x$email, as.character(x$when))
}

##' @export
print.git_signature <- function(x, ...) {
    cat(format(x, ...), "\n", sep = "")
    invisible(x)
}
