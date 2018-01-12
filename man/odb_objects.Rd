% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/odb.R
\name{odb_objects}
\alias{odb_objects}
\title{List all objects available in the database}
\usage{
odb_objects(repo = NULL)
}
\arguments{
\item{repo}{a \code{\linkS4class{git_repository}} object, or a
path to a repository, or \code{NULL}.  If the \code{repo}
argument is \code{NULL}, the repository is searched for with
\code{\link{discover_repository}} in the current working
directory.}
}
\value{
A data.frame with the following columns:
\describe{
  \item{sha}{The sha of the object}
  \item{type}{The type of the object}
  \item{len}{The length of the object}
}
}
\description{
List all objects available in the database
}
\examples{
\dontrun{
## Create a directory in tempdir
path <- tempfile(pattern="git2r-")
dir.create(path)

## Initialize a repository
repo <- init(path)
config(repo, user.name="Alice", user.email="alice@example.org")

## Create a file, add and commit
writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
           con = file.path(path, "test.txt"))
add(repo, "test.txt")
commit(repo, "Commit message 1")

## Create tag
tag(repo, "Tagname", "Tag message")

## List objects in repository
odb_objects(repo)
}
}