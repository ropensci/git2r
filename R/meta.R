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

##' Optimise a vector for storage in to a git repository and add meta data
##' @param x the vector
##' @noRd
meta <- function(x) {
  UseMethod("meta")
}

meta.character <- function(x) {
  attr(x, "meta") <- "    class: character"
  return(x)
}

meta.integer <- function(x) {
  attr(x, "meta") <- "    class: integer"
  return(x)
}

meta.numeric <- function(x) {
  attr(x, "meta") <- "    class: numeric"
  return(x)
}

meta.factor <- function(x) {
  z <- as.integer(x)
  attr(z, "meta") <- paste(
    "    class: factor\n    levels:",
    paste("        -", levels(x), collapse = "\n"),
    sep = "\n"
  )
  return(z)
}
