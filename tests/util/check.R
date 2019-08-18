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

empty_named_list <- function() {
    structure(list(), .Names = character(0))
}

## Raise an error if the error message doesn't match.
check_error <- function(current, target, exact = FALSE) {
    if (isTRUE(exact)) {
        stopifnot(identical(current[[1]]$message, target))
    } else {
        stopifnot(length(grep(target, current[[1]]$message)) > 0)
    }

    invisible(NULL)
}
