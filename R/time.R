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

##' @export
as.character.git_time <- function(x,  origin = "1970-01-01", tz = "GMT", ...) {
    as.character(as.POSIXct(x, origin = origin, tz = tz))
}

##' @export
as.POSIXct.git_time <- function(x, origin = "1970-01-01", tz = "GMT", ...) {
    as.POSIXct(x$time, origin = origin, tz = tz)
}

##' @export
print.git_time <- function(x,  origin = "1970-01-01", tz = "GMT", ...) {
    cat(sprintf("%s\n", as.character(x, origin = origin, tz = tz)))
    invisible(x)
}
