\name{repository}
\alias{repository}
\title{Open a repository}
\usage{
  repository(path)
}
\arguments{
  \item{path}{A path to an existing local git repository}
}
\value{
  A S4 \code{repository} object
}
\description{
  Open a repository
}
\examples{
\dontrun{
## Open an existing repository
repo <- repository('path/to/git2r')
}
}
\keyword{methods}

