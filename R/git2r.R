## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2023 The git2r contributors
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

##' git2r: R bindings to the libgit2 library
##'
##' git2r: R bindings to the libgit2 library.
##' @docType package
##' @aliases git2r-package
##' @name git2r
##' @useDynLib git2r, .registration=TRUE
NULL

##' Unload hook function
##'
##' @param libpath A character string giving the complete path to the
##' package.
##' @noRd
.onUnload <- function(libpath) {
    library.dynam.unload("git2r", libpath)
}
