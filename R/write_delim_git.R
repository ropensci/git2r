## Copyright (C) 2013-2018 The git2r contributors
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

##' Write a \code{data.frame} to a git repository
##'
##' This will create two files. The \code{".tsv"} file contains the raw data.
##' The \code{".yml"} contains the meta data on the columns in YAML format.
##' @param x the \code{data.frame}
##' @param file the name of the file without file extension. Can include a relative
##' path. It is relative to the "project" when set in the \code{repo}. Otherwise
##' it is relative to the root of the \code{repo}.
##' @param repo a \code{git_repository} object, created with
##' \code{\link{repository}}
##' @return The relative path to the file
##' @export
##' @include meta.R
##' @importFrom utils write.table
write_delim_git <- function(x, file, repo) {
  if (!inherits(x, "data.frame")) {
    stop("x is not a 'data.frame'")
  }
  if (grepl("\\..*$", basename(file))) {
    warning("file extensions are stripped")
    file <- file.path(dirname(file), gsub("\\..*$", "", basename(file)))
  }
  if (!inherits(repo, "git_repository")) {
    stop("repo is not a 'git_repository'")
  }

  raw_file <- file.path(workdir(repo), paste0(file, ".tsv"))
  meta_file <- file.path(workdir(repo), paste0(file, ".yml"))
  if (!dir.exists(dirname(raw_file))) {
    dir.create(dirname(raw_file), recursive = TRUE)
  }
  raw_data <- as.data.frame(lapply(x, meta), stringsAsFactors = FALSE)
  meta_data <- paste(
    colnames(x),
    vapply(raw_data, attr, "", which = "meta"),
    sep = ":\n"
  )
  writeLines(meta_data, meta_file)
  write.table(
    x = raw_data, file = raw_file, append = FALSE,
    quote = FALSE, sep = "\t", eol = "\n", dec = ".",
    row.names = FALSE, col.names = FALSE, fileEncoding = "UTF-8"
  )

  return(file)
}
