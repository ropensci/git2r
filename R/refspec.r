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

##' Create push refspec from arguments
##'
##' @param repo S4 class \code{git_repository}
##' @param name The remote's name. Default is NULL.
##' @param refspec The refspec to be pushed. Default is NULL.
##' @return Character vector with refspec
##' @keywords internal
get_refspec <- function(repo = NULL, remote = NULL, spec = NULL)
{
    stopifnot(is(object = repo, class2 = "git_repository"))

    if (is_detached(repo))
        stop("You are not currently on a branch.")

    spec
}
