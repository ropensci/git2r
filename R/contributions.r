#' Contributions_per_month
#'
#' Generates a ggplot chart of number of contributions to git repo per month
#' @param dir Path to git repo. Default is current working directory.
#' @param  title Default title for the plot is commits. Supply a new title if you desire one.
#' @param  breaks Default is \code{month}. Change to week or day as necessary.
#' @param  data_only = default is \code{FALSE}. Set to \code{TRUE} if you just require the \code{data.frame} output.
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
contributions <- function(dir = getwd(), title = "Commits", breaks = 'month', data_only = FALSE) {
  repo <- repository(dir)
    df <- do.call('rbind', lapply(commits(repo), function(x) {
    data.frame(name = x@author@name,
               when = as(x@author@when, 'POSIXct'))
}))

## Format data
df$month <- as.POSIXct(cut(df$when, breaks = breaks))
df <- ddply(df, ~month, nrow)
names(df) <- c('month', 'n')

if(data_only) {
  df
} else {
# return the plot
df$month <- as.Date(df$month)
ggplot(df, aes(x = month, y = n)) +
    geom_bar(stat = 'identity', fill = "steelblue") +
    scale_x_date('Month', labels = scales::date_format("%m-%Y")) +
    scale_y_continuous('# of commits') +
    ggtitle(sprintf("Commits on repo %s", basename(repo@path))) +
    labs(title = title)  + theme_gray()
}
  }


#' contribution_by_user
#'
#' See contributions to a Git repo by user
#' @param dir Path to git repo. Default is current working directory.
#' @param  breaks Default is \code{month}. Change to week or day as necessary.
#' @param  data_only = default is \code{FALSE}. Set to \code{TRUE} if you just require the \code{data.frame} output.
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
contribution_by_user <- function(dir = getwd(), breaks = 'month', data_only = FALSE) {
repo <- repository(dir)
df <- do.call('rbind', lapply(commits(repo), function(x) {
    data.frame(name = x@author@name,
               when = as(x@author@when, 'POSIXct'))
}))
df$month <- as.POSIXct(cut(df$when, breaks = breaks))


# Summarise the results
df_summary <- df %.%
group_by(name, month) %.%
summarise(counts = n()) %.%
arrange(month)



df_melted <-  melt(dcast(df_summary, name ~ month, value.var = "counts"), id.var = "name")
df_melted$variable <- as.Date(df_melted$variable)
names(df_melted)[2:3] <- c("month", "counts")

if(data_only) {
  df_melted
} else {
ggplot(df_melted, aes(month, counts, group = name, fill = name)) +
geom_bar(stat = "identity", position = "dodge", color = "black") +
expand_limits(y = 0) + xlab("Month") + ylab("Commits") +
ggtitle(sprintf("Commits on repo %s", basename(repo@path))) +
scale_x_date(labels = date_format("%b-%Y")) + theme_gray()
}

}
