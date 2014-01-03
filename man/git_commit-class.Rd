\docType{class}
\name{git_commit-class}
\alias{git_commit-class}
\alias{show,git_commit-method}
\title{Class \code{"git_commit"}}
\description{
  S4 class to handle a git commit.
}
\section{Slots}{
  \describe{ \item{author}{ An author signature }
  \item{committer}{ The committer signature }
  \item{summary}{ The short "summary" of a git commit
  message, comprising the first paragraph of the message
  with whitespace trimmed and squashed.  } \item{message}{
  The message of a commit } }
}

\section{Methods}{
  \describe{ \item{show}{\code{signature(object =
  "git_commit")}} }
}
\keyword{classes}
\keyword{methods}

