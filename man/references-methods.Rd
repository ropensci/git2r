\docType{methods}
\name{references-methods}
\alias{references}
\alias{references,git_repository-method}
\alias{references-methods}
\title{Get all references that can be found in a repository.}
\arguments{
  \item{object}{The repository \code{object}.}
}
\value{
  Character vector with references
}
\description{
  Get all references that can be found in a repository.
}
\examples{
\dontrun{
## Open an existing repository
repo <- repository('path/to/git2r')

## List all references in repository
references(repo)
}
}
\keyword{methods}

