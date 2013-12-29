\docType{class}
\name{reference-class}
\alias{reference-class}
\alias{show,reference-method}
\title{Class \code{"reference"}}
\description{
  S4 class to handle a git reference
}
\section{Slots}{
  \describe{ \item{name}{ The full name of the reference.
  } \item{type}{ Type of the reference, either direct
  (GIT_REF_OID == 1) or symbolic (GIT_REF_SYMBOLIC == 2) }
  \item{hex}{ 40 char hexadecimal string } \item{target}{ }
  \item{shorthand}{ The reference's short name } }
}

\section{Methods}{
  \describe{ \item{show}{\code{signature(object =
  "reference")}} }
}
\keyword{classes}
\keyword{methods}

