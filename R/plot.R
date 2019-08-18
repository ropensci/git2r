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

##' Plot commits over time
##'
##' @param x The repository to plot
##' @param breaks Default is \code{month}. Change to year, quarter,
##' week or day as necessary.
##' @param main Default title for the plot is "Commits on repo:" and
##' repository workdir basename. Supply a new title if you desire one.
##' @param ... Additional arguments affecting the plot
##' @importFrom graphics axis
##' @importFrom graphics barplot
##' @export
##' @examples
##' \dontrun{
##' ## Initialize repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- clone("https://github.com/ropensci/git2r.git", path)
##'
##' ## Plot commits
##' plot(repo)
##' }
plot.git_repository <- function(x,
                                breaks = c("month",
                                           "year",
                                           "quarter",
                                           "week",
                                           "day"),
                                main = NULL,
                                ...) {
    breaks <- match.arg(breaks)

    df <- contributions(x, breaks = breaks, by = "commits")
    tmp <- data.frame(when = seq(from = min(df$when),
                                 to   = max(df$when),
                                 by   = breaks),
                      n = 0)
    i <- match(df$when, tmp$when)
    tmp$n[i] <- df$n
    df <- tmp

    xlab <- sprintf("Time [%s]", breaks)
    ylab <- "Number of commits"
    if (is.null(main)) {
        if (is_bare(x)) {
            main <- "Commits"
        } else {
            main <- sprintf("Commits on repository: %s",
                            basename(workdir(x)))
        }
    }

    mp <- barplot(df$n, xlab = xlab, ylab = ylab, main = main, ...)
    axis(1, at = mp, labels = seq(min(df$when), max(df$when), breaks))
}
