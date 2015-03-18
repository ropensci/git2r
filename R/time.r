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

setAs(from="git_time",
      to="character",
      def=function(from)
      {
          as.character(as(from, "POSIXct"))
      }
)

setAs(from="git_time",
      to="POSIXct",
      def=function(from)
      {
          as.POSIXct(from@time + from@offset*60,
                     origin="1970-01-01",
                     tz="GMT")
      }
)

setAs(from = "POSIXlt",
      to   = "git_time",
      def=function(from)
      {
          if (is.null(from$gmtoff) || is.na(from$gmtoff)) {
              offset <- 0
          } else {
              offset <- from$gmtoff / 60
          }

          new("git_time",
              time   = as.numeric(from),
              offset = offset)
      }
)

##' Brief summary of \code{git_time}
##'
##' @aliases show,git_time-methods
##' @docType methods
##' @param object The time \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Brief summary of git_time from the default signature
##' default_signature(repo)@@when
##' }
setMethod("show",
          signature(object = "git_time"),
          function(object)
          {
              cat(sprintf("%s\n", as(object, "character")))
          }
)
