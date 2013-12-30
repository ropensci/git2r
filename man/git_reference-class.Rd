\docType{class}
\name{git_reference-class}
\alias{git_reference-class}
\alias{show,git_reference-method}
\title{Class \code{"git_reference"}}
\description{
  S4 class to handle a git reference
}
\section{Slots}{
  \describe{ \item{name}{ The full name of the reference.
  } \item{type}{ Type of the reference, either direct
  (GIT_REF_OID == 1) or symbolic (GIT_REF_SYMBOLIC == 2) }
  \item{hex}{ 40 char hexadecimal string } \item{target}{
  :TODO:DOCUMENTATION: } \item{shorthand}{ The reference's
  short name } }
}

\section{Methods}{
  \describe{ \item{show}{\code{signature(object =
  "git_reference")}} }
}
\keyword{classes}
\keyword{methods}

