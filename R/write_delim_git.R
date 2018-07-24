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
##' @template repo-param
##' @inheritParams add
##' @return NULL (invisible)
##' @export
##' @importFrom utils write.table
write_delim_git <- function(x, file, repo = ".", force = FALSE) {
    if (!inherits(x, "data.frame")) {
        stop("x is not a 'data.frame'")
    }
    repo <- lookup_repository(repo)
    if (is_data_repo(repo)) {
        file <- file.path(workdir(repo), file)
    }
    if (grepl("\\..*$", basename(file))) {
        warning("file extensions are stripped")
    }
    file <- clean_data_path(file)

    if (!dir.exists(dirname(file["raw_file"]))) {
        dir.create(dirname(file["raw_file"]), recursive = TRUE)
    }
    raw_data <- as.data.frame(lapply(x, meta), stringsAsFactors = FALSE)
    meta_data <- paste(
        colnames(x),
        vapply(raw_data, attr, "", which = "meta"),
        sep = ":\n"
    )
    writeLines(meta_data, file["meta_file"])
    write.table(
        x = raw_data, file = file["raw_file"], append = FALSE,
        quote = FALSE, sep = "\t", eol = "\n", dec = ".",
        row.names = FALSE, col.names = FALSE, fileEncoding = "UTF-8"
    )
    add(repo, path = file, force = force)

    return(invisible(NULL))
}

##' Read a \code{data.frame} from a git repository
##'
##' @template repo-param
##' @inheritParams write_delim_git
##' @return The \code{data.frame}
##' @export
##' @importFrom utils read.table
read_delim_git <- function(file, repo = ".") {
    repo <- lookup_repository(repo)
    if (is_data_repo(repo)) {
        file <- file.path(workdir(repo), file)
    }
    file <- clean_data_path(file)

    if (!all(file.exists(file))) {
        stop("raw file and/or meta file missing")
    }
    meta_data <- readLines(file["meta_file"])
    meta_cols <- grep("^\\S*:$", meta_data)
    col_names <- gsub(":", "", meta_data[meta_cols])
    raw_data <- read.table(
        file = file["raw_file"], header = FALSE,
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

##' Optimise a vector for storage in to a git repository and add meta data
##' @param x the vector
##' @export
meta <- function(x) {
    UseMethod("meta")
}

##' @export
meta.character <- function(x) {
    attr(x, "meta") <- "    class: character"
    return(x)
}

##' @export
meta.integer <- function(x) {
    attr(x, "meta") <- "    class: integer"
    return(x)
}

##' @export
meta.numeric <- function(x) {
    attr(x, "meta") <- "    class: numeric"
    return(x)
}

##' @export
meta.factor <- function(x) {
    z <- as.integer(x)
    attr(z, "meta") <- paste(
        "    class: factor\n    levels:",
        paste("        -", levels(x), collapse = "\n"),
        sep = "\n"
    )
    return(z)
}

##' Check if object is a data repository
##' @param object the object to check
##' @return TRUE is a data \code{git_repository}, else FALSE
##' @seealso repo init
##' @export
is_data_repo <- function(object) {
    inherits(object, "git_repository") && !is.null(object$project)
}

##' Clean the data path
##' Strips any file extension from the path and adds the ".tsv" and ".yml" file extensions
##' @param path the paths
##' @return a named vector with "raw_file" and "meta_file", refering to the ".tsv" and ".yml" files
##' @noRd
clean_data_path <- function(path) {
    dir_name <- dirname(path)
    not_root <- dir_name != "."
    path <- gsub("\\..*$", "", basename(path))
    if (any(not_root)) {
        path[not_root] <- file.path(dir_name[not_root], path[not_root])
    }
    path <- normalizePath(unique(path), mustWork = FALSE)
    c(raw_file = paste0(path, ".tsv"), meta_file = paste0(path, ".yml"))
}
