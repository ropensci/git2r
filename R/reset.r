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

##' Reset current HEAD to the specified state
##'
##' @rdname reset-methods
##' @docType methods
##' @param commit The \code{\linkS4class{git_commit}} to which the
##' HEAD should be moved to.
##' @param reset_type Kind of reset operation to perform. 'soft' means
##' the Head will be moved to the commit. 'mixed' reset will trigger a
##' 'soft' reset, plus the index will be replaced with the content of
##' the commit tree. 'hard' reset will trigger a 'mixed' reset and the
##' working directory will be replaced with the content of the index.
##' @param msg The one line long message to the reflog. The default
##' value is "reset: moving".
##' @param who The identity that will be used to populate the
##' reflog entry. Default is the default signature.
##' @return invisible NULL
##' @keywords methods
##' @include S4_classes.r
setGeneric("reset",
           signature = c("commit"),
           function(commit,
                    reset_type = c("soft", "mixed", "hard"),
                    msg        = "reset: moving",
                    who        = default_signature(commit@repo))
           standardGeneric("reset"))

##' @rdname reset-methods
##' @export
setMethod("reset",
          signature(commit = "git_commit"),
          function (commit,
                    reset_type,
                    msg,
                    who)
          {
              reset_type <- switch(match.arg(reset_type),
                                   soft  = 1L,
                                   mixed = 2L,
                                   hard  = 3L)

              invisible(.Call(git2r_reset,
                              commit,
                              reset_type,
                              msg,
                              who))

          }
)
