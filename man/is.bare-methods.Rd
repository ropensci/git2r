\docType{methods}
\name{is.bare-methods}
\alias{is.bare}
\alias{is.bare,git_repository-method}
\alias{is.bare-methods}
\title{Check if repository is bare}
\arguments{
  \item{object}{The \code{object} to check if it's a bare
  repository}
}
\value{
  TRUE if bare repository, else FALSE
}
\description{
  Check if repository is bare
}
\examples{
\dontrun{
## Open an existing repository
repo <- repository('path/to/git2r')

## Check if it's a bare repository
is.bare(repo)
}
}
\keyword{methods}

