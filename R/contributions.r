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

##' Contributions
##'
##' See contributions to a Git repo
##' @rdname contributions-methods
##' @docType methods
##' @param repo The repository.
##' @param breaks Default is \code{month}. Change to week or day as necessary.
##' @param by Contributions by "commits" or "author". Default is "commits".
##' @return A \code{data.frame} with contributions.
##' @keywords methods
##' @include repository.r
##' @author Karthik Ram \email{karthik.ram@@gmail.com}
##' @author Stefan Widgren \email{stefan.widgren@@gmail.com}
##' @examples \dontrun{
##' ## If current working dir is a git repo
##' contributions()
##' contributions(breaks = "week")
##' contributions(breaks = "day")
##'
##' ## If the path is somewhere else
##' contributions("/path/to/repo")
##'}
setGeneric("contributions",
           signature = "repo",
           function(repo,
                    breaks = c("month", "week", "day"),
                    by = c("commits", "author"))
           standardGeneric("contributions"))

##' @rdname contributions-methods
##' @export
setMethod("contributions",
          signature(repo = "missing"),
          function (repo, breaks, by)
          {
              ## Try current working directory
              contributions(getwd(), breaks = breaks, by = by)
          }
)

##' @rdname contributions-methods
##' @export
setMethod("contributions",
          signature(repo = "character"),
          function (repo, breaks, by)
          {
              contributions(repository(repo), breaks = breaks, by = by)
          }
)

##' @rdname contributions-methods
##' @export
setMethod("contributions",
          signature(repo = "git_repository"),
          function (repo, breaks, by)
          {
              breaks <- match.arg(breaks)
              by <- match.arg(by)

              df <- as(repo, "data.frame")
              df$when <- as.POSIXct(cut(df$when, breaks = breaks))

              if(identical(by, "commits")) {
                  df <- as.data.frame(table(df$when))
                  names(df) <- c("when", "n")
                  df$when <- as.Date(df$when)
                  return(df)
              }

              ## Create an index and tabulate
              df$index <- paste0(df$when, df$author, df$email)
              count <- as.data.frame(table(df$index),
                                     stringsAsFactors=FALSE)
              names(count) <- c("index", "n")

              ## Match counts and clean result
              df$n <- count$n[match(df$index, count$index)]
              df <- unique(df[, c("when", "author", "n")])
              df$when <- as.Date(substr(as.character(df$when), 1, 10))
              df <- df[order(df$when, df$author),]
              row.names(df) <- NULL
              return(df)
          }
)
