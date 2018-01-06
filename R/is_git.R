#' Is the path a git repository
#' Checks if a '.git' subdirectory exists in 'path'
#' @param path the path to check
#' @export
#' @return A logical vector with the same length as path
#' @importFrom utils file_test
#' @template thierry
is.git <- function(path){
  file_test("-d", paste(path, ".git", sep = "/"))
}
