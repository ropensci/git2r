#' Remove all the files in a path of a git repository
#'
#' @inheritParams write_delim_git
#' @inheritParams base::list.files
#' @name remove_files_git
#' @rdname remove_files_git
#' @exportMethod remove_files_git
#' @docType methods
#' @importFrom methods setGeneric
#' @include list_files_git.r
#' @template thierry
setGeneric(
  name = "remove_files_git",
  def = function(connection, pattern = NULL, ...){
    standard.generic("remove_files_git")
  }
)

#' @rdname remove_files_git
#' @aliases remove_files_git,git_connection-methods
#' @importFrom methods setMethod
setMethod(
  f = "remove_files_git",
  signature = signature(connection = "ANY"),
  definition = function(connection, pattern = NULL, ...){
    this.connection <- git_connection(
      repo.path = connection,
      local.path = list(...)$path,
      ...
    )
    remove_files_git(connection = this.connection, pattern = pattern)
  }
)

#' @rdname remove_files_git
#' @aliases remove_files_git,git_connection-methods
#' @importFrom methods setMethod
setMethod(
  f = "remove_files_git",
  signature = signature(connection = "gitConnection"),
  definition = function(connection, pattern = NULL, ...){
    to.remove <- list_files_git(
      connection = connection,
      pattern = pattern,
      full.names = TRUE
    )

    success <- file.remove(to.remove)

    if (length(success) > 0 && !all(success)) {
      stop(
        "Error cleaning existing files in the git repository. Repository: '",
        connection@Repository@path, "', Path: '", connection@LocalPath,
        "', pattern: '", pattern, "'"
      )
    }
    if (any(success)) {
      to.stage <- gsub(
        paste0("^", connection@Repository@path, "/"),
        "",
        to.remove[success]
      )
      add(repo = connection@Repository, path = to.stage)
    }
    return(invisible(TRUE))
  }
)
