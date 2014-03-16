\docType{class}
\name{git_time-class}
\alias{coerce,git_time,character-method}
\alias{git_time-class}
\title{Class \code{"git_time"}}
\description{
S4 class to handle a git time in a signature
}
\section{Slots}{
  \describe{ \item{time}{ Time in seconds from epoch }
  \item{offset}{ Timezone offset, in minutes } }
}

\section{Methods}{
  \describe{ \item{show}{\code{signature(object =
  "git_time")}} }
}
\keyword{classes}
\keyword{methods}

