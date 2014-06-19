## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2014 The git2r contributors
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

##' Class \code{git_note}
##'
##' @title S4 class to handle a git note
##' @section Slots:
##' \describe{
##'   \item{hex}{
##'     40 char hexadecimal string
##'   }
##'   \item{note}{
##'     The note message
##'   }
##'   \item{repo}{
##'     The S4 class git_repository that contains the note
##'   }
##' }
##' @name git_note-class
##' @docType class
##' @keywords classes
##' @keywords methods
##' @include repository.r
##' @export
setClass("git_note",
         slots = c(hex  = "character",
                   note = "character",
                   repo = "git_repository"))
