#' Contributions_per_month
#'
#' Generates a ggplot chart of number of contributions to git repo per month
#' @param dir Path to git repo. Default is current working directory.
#' @param  title Default title for the plot is commits. Supply a new title if you desire one.
#' @param  breaks Default is \code{month}. Change to week or day as necessary.
#' @return A \code{data.frame} with contributions.
#' @importFrom scales date_format
#' @import ggplot2
#' @importFrom plyr ddply
#' @export
#' @author Karthik Ram \email{karthik.ram@@gmail.com}
#' @examples \dontrun{
#' # If current working dir is a git repo
#' # contributions()
#' # contributions(breaks = "week")
#' # contributions(breaks = "day")
#' # If the path is somewhere else
#' # contributions(dir = "/path/to/repo")
#' # If you need just the data, not the plot
#' contributions(data_only = TRUE)
#'}
contributions <- function(dir = getwd(), breaks = 'month') {
    repo <- repository(dir)
    df <- do.call('rbind', lapply(commits(repo), function(x) {
        data.frame(name = x@author@name,
                   when = as(x@author@when, 'POSIXct'))
    }))

    ## Format data
    df$month <- as.POSIXct(cut(df$when, breaks = breaks))
    df <- ddply(df, ~month, nrow)
    names(df) <- c('month', 'n')

    df
}


#' contribution_by_user
#'
#' See contributions to a Git repo by user
#' @param dir Path to git repo. Default is current working directory.
#' @param  breaks Default is \code{month}. Change to week or day as necessary.
#' @return A \code{data.frame} with contribtions by user.
#' @import dplyr
#' @importFrom scales date_format
#' @import ggplot2
#' @importFrom reshape2 melt dcast
#' @export
#' @author Karthik Ram \email{karthik.ram@@gmail.com}
#' @examples \dontrun{
#' contribution_by_user()
#' # To just download the data to use separately
#' contribution_by_user(data_only = TRUE)
#'}
contribution_by_user <- function(dir = getwd(), breaks = 'month') {
    repo <- repository(dir)
    df <- do.call('rbind', lapply(commits(repo), function(x) {
        data.frame(name = x@author@name,
                   when = as(x@author@when, 'POSIXct'))
    }))
    df$month <- as.POSIXct(cut(df$when, breaks = breaks))

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
