#' Get the SHA of the files at the HEAD
#' @inheritParams write_delim_git
#' @name git_sha
#' @rdname git_sha
#' @exportMethod git_sha
#' @docType methods
#' @importFrom methods setGeneric
#' @include git_connection.r
#' @template thierry
setGeneric(
  name = "git_sha",
  def = function(file, connection, ...){
    standard.generic("git_sha")
  }
)

#' @rdname git_sha
#' @aliases git_sha,git_connection-methods
#' @importFrom methods setMethod
setMethod(
  f = "git_sha",
  signature = signature(connection = "ANY"),
  definition = function(file, connection, ...){
    this.connection <- git_connection(repo.path = connection, ...)
    git_sha(file = file, connection = this.connection)
  }
)

#' @rdname git_sha
#' @aliases git_sha,git_connection-methods
#' @importFrom methods setMethod
#' @importFrom assertthat assert_that is.string
#' @importFrom utils read.table
setMethod(
  f = "git_sha",
  signature = signature(connection = "gitConnection"),
  definition = function(file, connection, ...){
    assert_that(is.string(file))

    old.wd <- getwd()
    setwd(connection@Repository@path)
    blobs <- system(
      paste(
        "git ls-tree -r HEAD",
        connection@LocalPath
      ),
      intern = TRUE
    )
    setwd(old.wd)
    blobs <- read.table(
      textConnection(paste(blobs, collapse = "\n")),
      header = FALSE,
      sep = "\t",
      col.names = c("SHA", "Path")
    )
    blobs$File <- basename(blobs$Path)
    blobs <- blobs[blobs$File %in% file, ]
    blobs$Path <- dirname(blobs$Path)
    blobs$SHA <- gsub("^.*blob ", "", blobs$SHA)

    return(blobs)
  }
)
