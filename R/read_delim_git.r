#' Read a tab delimited file from a git repository
#' @param file the name of the file
#' @param connection The path of a git repository or a \code{git_connection}
#'    object
#' @param ... parameters passed to \code{git_connection()} when
#'    \code{connection} is a path
#' @name read_delim_git
#' @rdname read_delim_git
#' @exportMethod read_delim_git
#' @docType methods
#' @importFrom methods setGeneric
#' @include git_connection.r
#' @template thierry
setGeneric(
  name = "read_delim_git",
  def = function(file, connection, ...){
    standard.generic("read_delim_git")
  }
)

#' @rdname read_delim_git
#' @aliases read_delim_git,git_connection-methods
#' @importFrom methods setMethod
setMethod(
  f = "read_delim_git",
  signature = signature(connection = "ANY"),
  definition = function(file, connection, ...){
    this.connection <- git_connection(repo.path = connection, ...)
    read_delim_git(file = file, connection = this.connection)
  }
)

#' @rdname read_delim_git
#' @aliases read_delim_git,git_connection-methods
#' @importFrom methods setMethod
#' @importFrom utils read.delim
#' @importFrom assertthat assert_that is.string
setMethod(
  f = "read_delim_git",
  signature = signature(connection = "git_connection"),
  definition = function(file, connection, ...){
    assert_that(is.string(file))

    filename <- normalizePath(
        sprintf(
        "%s/%s/%s",
        connection@Repository@path,
        connection@LocalPath,
        file
      ),
      mustWork = TRUE
    )
    return(read.delim(filename, stringsAsFactors = FALSE))
  }
)
