## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2014  Stefan Widgren
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, version 2 of the License.
##
## git2r is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.

##' Contributions
##'
##' See contributions to a Git repo
##' @param dir Path to git repo. Default is current working directory.
##' @param breaks Default is \code{month}. Change to week or day as necessary.
##' @param by Contributions by 'commits' or 'user'. Default is 'commits'.
##' @return A \code{data.frame} with contributions.
##' @importFrom plyr ddply
##' @importFrom reshape2 melt dcast
##' @export
##' @author Karthik Ram \email{karthik.ram@@gmail.com}
##' @author Stefan Widgren \email{stefan.widgren@@gmail.com}
##' @examples \dontrun{
##' # If current working dir is a git repo
##' # contributions()
##' # contributions(breaks = "week")
##' # contributions(breaks = "day")
##' # If the path is somewhere else
##' # contributions(dir = "/path/to/repo")
##'}
contributions <- function(dir = getwd(),
                          breaks = c('month', 'week', 'day'),
                          by = c('commits', 'user'))
{
    breaks <- match.arg(breaks)
    by <- match.arg(by)

    repo <- repository(dir)
    df <- do.call('rbind', lapply(commits(repo), function(x) {
        data.frame(name = x@author@name,
                   when = as(x@author@when, 'POSIXct'))
    }))

    if(identical(by, 'commits')) {
        ## Format data
        df$when <- as.POSIXct(cut(df$when, breaks = breaks))
        df <- ddply(df, ~when, nrow)
        names(df) <- c('when', 'n')
        return(df)
    }

    ## Summarise the results
    df_summary <- df %.%
        group_by(name, month) %.%
        summarise(counts = n()) %.%
        arrange(month)

    df_melted <-  melt(dcast(df_summary, name ~ month, value.var = "counts"), id.var = "name")
    df_melted$variable <- as.Date(df_melted$variable)
    names(df_melted)[2:3] <- c("month", "counts")

    df_melted
}
