\docType{methods}
\name{add-methods}
\alias{add}
\alias{add-methods}
\title{Add file(s) to index}
\usage{
add(object, path)
}
\arguments{
  \item{object}{The repository \code{object}.}

  \item{path}{character vector with filenames to add. The
  path must be relative to the repository's working
  filder.}
}
\value{
invisible(NULL)
}
\description{
Add file(s) to index
}
\examples{
\dontrun{
## Open an existing repository
repo <- repository('path/to/git2r')

## Add file repository
add(repo, 'file-to-add')
}
}
\keyword{methods}

