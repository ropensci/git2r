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

##' Compile time options for libgit2.
##'
##' @return A list with threads, https and ssh set to TRUE/FALSE.
##' @keywords methods
##' @export
##' @examples
##' libgit2_features()
libgit2_features <- function() {
    .Call(git2r_libgit2_features)
}

##' Version of the libgit2 library
##'
##' Version of the libgit2 library that the bundled source code is
##' based on
##' @return A list with major, minor and rev
##' @keywords methods
##' @export
##' @examples
##' libgit2_version()
libgit2_version <- function() {
    .Call(git2r_libgit2_version)
}

##' SHA of the libgit2 library
##'
##' SHA of the libgit2 library that the bundled source code is based
##' on
##' @return The 40 character hexadecimal string of the SHA-1
##' @keywords methods
##' @export
##' @examples
##' libgit2_sha()
libgit2_sha <- function() "7175222ce63ef61749cc00a1f29d11d098bd28a8"

##' Set the SSL certificate-authority locations
##'
##' @note Either parameter may be 'NULL', but not both.
##' @param filename Location of a file containing several certificates
##' concatenated together. Default NULL.
##' @param path Location of a directory holding several certificates,
##' one per file. Default NULL.
##' @return invisible(NULL)
##' @keywords methods
##' @export
ssl_cert_locations <- function(filename = NULL, path = NULL)
{
    invisible(.Call(git2r_ssl_cert_locations, filename, path))
}
