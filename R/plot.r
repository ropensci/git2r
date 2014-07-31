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

##' Plot contributions over time
##'
##' @rdname plot-methods
##' @aliases plot
##' @aliases plot-methods
##' @docType methods
##' @param x The repository to plot
##' @param breaks Default is \code{month}. Change to week or day as
##' necessary.
##' @param by Contributions by "commits" or "author". Default is
##' "commits".
##' @param title Default title for the plot is "Commits on repo:" and
##' repository workdir basename. Supply a new title if you desire one.
##' @return A \code{ggplot} chart
##' @keywords methods
##' @import ggplot2
##' @importFrom scales date_format
##' @include repository.r
##' @export
setMethod("plot",
          signature(x = "git_repository"),
          function (x,
                    breaks = c("month", "week", "day"),
                    by = c("commits", "author"),
                    title = "Commits")
          {
              breaks = match.arg(breaks)
              by = match.arg(by)

              df <- contributions(x, breaks = breaks, by = by)

              xlab <- switch(breaks,
                             "month" = "Month",
                             "week"  = "Week",
                             "day"   = "Day")
              ylab <- "Number of commits"
              title <- sprintf("Commits on repository: %s", basename(workdir(x)))

              if (identical(by, "commits")) {
                  ggplot(df, aes(x = when, y = n)) +
                      geom_bar(stat = "identity", fill = "steelblue") +
                      scale_x_date(xlab, labels = scales::date_format("%m-%Y")) +
                      scale_y_continuous(ylab) +
                      ggtitle(title) + theme_gray()
              } else {
                  ggplot(df, aes(x = when, y = n, group = author, fill = author)) +
                      geom_bar(stat = "identity", color = "black") +
                      scale_x_date(xlab, labels = scales::date_format("%m-%Y")) +
                      scale_y_continuous(ylab) +
                      ggtitle(title) + theme_gray()
              }
          }
)
