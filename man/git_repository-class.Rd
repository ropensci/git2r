\docType{class}
\name{git_repository-class}
\alias{git_repository-class}
\alias{show,git_repository-method}
\alias{summary,git_repository-method}
\title{Class \code{"git_repository"}}
\description{
  S4 class to handle a git repository
}
\section{Slots}{
  \describe{ \item{repo}{ External pointer to a git
  repository } \item{walker}{ External pointer to a git
  revision walker } }
}

\section{Methods}{
  \describe{ \item{is.bare}{\code{signature(object =
  "git_repository")}}
  \item{is.empty}{\code{signature(object =
  "git_repository")}} \item{show}{\code{signature(object =
  "git_repository")}} \item{summary}{\code{signature(object
  = "git_repository")}} }
}
\keyword{classes}
\keyword{methods}

