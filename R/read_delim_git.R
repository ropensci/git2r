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

##' Read a \code{data.frame} from a git repository
##'
##' @inheritParams write_delim_git
##' @export
##' @importFrom utils read.table
read_delim_file <- function(file, repo) {
  file <- gsub("\\..*$", "", file)
  if (!inherits(repo, "git_repository")) {
    stop("repo is not a 'git_repository'")
  }
  raw_file <- sprintf("%s/%s.tsv", dirname(repo$path), file)
  meta_file <- sprintf("%s/%s.yml", dirname(repo$path), file)
  if (!file.exists(raw_file) || !file.exists(meta_file)) {
    stop("raw file and/or meta file missing")
  }
  meta_data <- readLines(meta_file)
  meta_cols <- grep("^\\S*:$", meta_data)
  col_names <- gsub(":", "", meta_data[meta_cols])
  raw_data <- read.table(
    file = raw_file, header = FALSE,
    sep = "\t", quote = "", dec = ".",
    as.is = TRUE, col.names = col_names
  )

  col_classes <- gsub(" {4}class: (.*)", "\\1", meta_data[meta_cols + 1])
  col_factor <- which(col_classes == "factor")
  level_rows <- grep("^ {8}- .*$", meta_data)
  level_value <- gsub("^ {8}- (.*)$", "\\1", meta_data[level_rows])
  level_id <- cumsum(c(TRUE, diff(level_rows) > 1))
  col_factor_level <- vapply(
    seq_along(col_factor),
    function(id) {
      list(level_value[level_id == id])
    },
    list(character(0))
  )
  names(col_factor_level) <- col_names[col_factor]
  for (id in names(col_factor_level)) {
    raw_data[[id]] <- factor(
      raw_data[[id]],
      levels = seq_along(col_factor_level[[id]]),
      labels = col_factor_level[[id]]
    )
  }

  return(raw_data)
}
