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

##' @export
as.character.git_time <- function(x, ...) {
    as.character(as.POSIXct(x))
}

##' @export
as.POSIXct.git_time <- function(x, ...) {
    as.POSIXct(x$time + x$offset*60, origin = "1970-01-01", tz = "GMT")
}

## setAs(from = "POSIXlt",
##       to   = "git_time",
##       def=function(from)
##       {
##           if (is.null(from$gmtoff) || is.na(from$gmtoff)) {
##               offset <- 0
##           } else {
##               offset <- from$gmtoff / 60
##           }

##           structure(list(time = as.numeric(from), offset = offset),
##                     class = "git_time")
##       }
## )

##' @export
print.git_time <- function(x, ...) {
    cat(sprintf("%s\n", as.character(x)))
    invisible(x)
}
