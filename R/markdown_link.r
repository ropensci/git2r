## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2014 The git2r contributors
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License, version 2,
## as published by the Free Software Foundation.
##
## git2r is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program; if not, write to the Free Software Foundation, Inc.,
## 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#' @include S4-classes.r
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
