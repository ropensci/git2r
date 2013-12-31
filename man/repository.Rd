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
  A S4 \code{git_repository} object
}
\description{
  Open a repository
}
\examples{
\dontrun{
## Open an existing repository
repo <- repository('path/to/git2r')

## Brief summary of repository
repo

## Summary of repository
summary(repo)

## Workdir of repository
workdir(repo)

## Check if repository is bare
is.bare(repo)

## Check if repository is empty
is.empty(repo)

## List all references in repository
references(repo)

## List all branches in repository
branches(repo)

## Get HEAD of repository
head(repo)

## Check if HEAD is head
is.head(head(repo))

## Check if HEAD is local
is.local(head(repo))

## List all tags in repository
tags(repo)
}
}
\keyword{methods}

