 #' @include commit.r
#' @name markdown_link
#'
#' @title Generate a markdown link for current commit
#'
#' @description This function would be ideal to use when writing manscripts with markdown. 
#' By adding a bit of text like "All the files at the current state of the manuscript can by found in commit" followed by \code{markdown_link(repo)},
#' One can maintain some degree of provenance.
#' @param repo Path to local git repository
#' @export 
#' @examples \dontrun{
#' markdown_link(".") # or path to repo
#'}
markdown_link <- function(repo = ".") {
repo <- repository(repo)	
commit <- git2r::head(repo)@hex
short_hash <- substr(commit, 1, 6)
remote_link <- paste0(remote_url(repo), "/commit/", commit)
sprintf("[%s](%s)", short_hash, remote_link)
}