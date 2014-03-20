\name{contribution_by_user}
\alias{contribution_by_user}
\title{contribution_by_user}
\usage{
contribution_by_user(dir = getwd(), breaks = "month", data_only = FALSE)
}
\arguments{
  \item{dir}{Path to git repo. Default is current working
  directory.}

  \item{breaks}{Default is \code{month}. Change to week or
  day as necessary.}

  \item{data_only}{= default is \code{FALSE}. Set to
  \code{TRUE} if you just require the \code{data.frame}
  output.}
}
\description{
See contributions to a Git repo by user
}
\examples{
\dontrun{
contribution_by_user()
# To just download the data to use separately
contribution_by_user(data_only = TRUE)
}
}
\author{
Karthik Ram \email{karthik.ram@gmail.com}
}

