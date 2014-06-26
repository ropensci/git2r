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

##' Class \code{"git_reflog_entry"}
##'
##' @title S4 class to handle a git reflog entry.
##' @section Slots:
##' \describe{
##'   \item{hex}{
##'     40 char hexadecimal string
##'   }
##'   \item{message}{
##'     The log message of the entry
##'   }
##'   \item{committer}{
##'     The committer signature
##'   }
##'   \item{refname}{
##'     Name of the reference
##'   }
##'   \item{repo}{
##'     The S4 class git_repository that contains the reflog entry
##'   }
##' }
##' @name git_reflog_entry-class
##' @docType class
##' @keywords classes
##' @keywords methods
##' @include repository.r
##' @include signature.r
##' @export
setClass("git_reflog_entry",
         slots=c(hex       = "character",
                 message   = "character",
                 index     = "integer",
                 committer = "git_signature",
                 refname   = "character",
                 repo      = "git_repository"))
