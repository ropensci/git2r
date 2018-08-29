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
##' @param sorting a vector of column names defining which columns to use for
##' sorting \code{x} and in what order to use them. Defaults to
##' \code{colnames(x)}
##' @param override Ignore existing meta data. This is required when new
##' variables are added or variables are deleted. Setting this to TRUE can
##' potentially lead to large diffs. Defaults to FALSE.
##' @param stage immediatly stage the changes
##' @inheritParams add
##' @inheritParams meta
##' @return a named vector with the hashes of the files. The names contains the
##' files with their paths relative to the root of the git_repository.
##' @export
##' @importFrom utils tail write.table
write_delim_git <- function(
    x, file, repo = ".", sorting, override = FALSE, stage = FALSE,
    optimize = TRUE, force = FALSE
) {
    if (!inherits(x, "data.frame")) {
        stop("x is not a 'data.frame'")
    }
    repo <- lookup_repository(repo)
    if (!missing(sorting)) {
        if (length(sorting) == 0) {
            stop("at least one variable is required for sorting")
        }
        if (!all(sorting %in% colnames(x))) {
            stop("use only variables of 'x' for sorting")
        }
    }
    if (grepl("\\..*$", basename(file))) {
        warning("file extensions are stripped")
    }
    file <- file.path(workdir(repo), file)
    file <- clean_data_path(file)

    if (!file.exists(dirname(file["raw_file"]))) {
        dir.create(dirname(file["raw_file"]), recursive = TRUE)
    }
    raw_data <- as.data.frame(
        lapply(x, meta, optimize = optimize),
        stringsAsFactors = FALSE
    )
    meta_data <- paste(
        colnames(x),
        vapply(raw_data, attr, "", which = "meta"),
        sep = ":\n"
    )
    names(meta_data) <- colnames(x)
    if (override || !file.exists(file["meta_file"])) {
        if (missing(sorting)) {
            sorting <- colnames(x)
        }
        to_sort <- colnames(x) %in% sorting
        meta_data <- meta_data[c(sorting, colnames(x)[!to_sort])]
        meta_data[sorting] <- paste0(meta_data[sorting], "\n    sort")
        if (optimize) {
            store_meta_data <- c(meta_data, "optimized")
        } else {
            store_meta_data <- c(meta_data, "verbose")
        }
        writeLines(store_meta_data, file["meta_file"])
    } else {
        old_meta_data <- readLines(file["meta_file"])
        if (tail(old_meta_data, 1) == "verbose") {
            if (optimize) {
                stop("old data was stored verbose")
            }
        } else if (tail(old_meta_data, 1) == "optimized") {
            if (!optimize) {
                stop("old data was stored optimized")
            }
        } else {
            stop("error in existing metadata")
        }
        meta_cols <- grep("^\\S*:$", old_meta_data)
        positions <- cbind(
            start = meta_cols,
            end = c(tail(meta_cols, -1) - 1, length(old_meta_data) - 1)
        )
        old_meta_data <- apply(
            positions,
            1,
            function(i) {
                paste(old_meta_data[i["start"]:i["end"]], collapse = "\n")
            }
        )
        if (missing(sorting)) {
            sorting <- grep(".*sort", old_meta_data)
            sorting <- gsub("(\\S*?):\n.*", "\\1", old_meta_data)[sorting]
            if (!all(sorting %in% colnames(x))) {
                stop("new data lacks old sorting variable, use override = TRUE")
            }
        }
        to_sort <- colnames(x) %in% sorting
        meta_data <- meta_data[c(sorting, colnames(x)[!to_sort])]
        meta_data[sorting] <- paste0(meta_data[sorting], "\n    sort")
        meta_data <- compare_meta(meta_data, old_meta_data)
    }
    # order the variables
    raw_data <- raw_data[gsub("(\\S*?):.*", "\\1", meta_data)]
    # order the observations
    if (anyDuplicated(raw_data[sorting])) {
        warning(
"sorting results in ties. Add extra sorting variables to ensure small diffs."
        )
    }
    raw_data <- raw_data[do.call(order, raw_data[sorting]), ]
    write.table(
        x = raw_data, file = file["raw_file"], append = FALSE,
        quote = !optimize, sep = "\t", eol = "\n", dec = ".",
        row.names = FALSE, col.names = !optimize, fileEncoding = "UTF-8"
    )
    if (stage) {
        add(repo, path = file, force = force)
    }

    hashes <- hashfile(file)
    names(hashes) <- gsub(paste0("^", workdir(repo), "/"), "", file)

    return(hashes)
}

compare_meta <- function(meta_data, old_meta_data) {
    if (length(old_meta_data) != length(meta_data)) {
        stop(
            call. = FALSE,
            "old data has different number of variables, use override = TRUE"
        )
    }
    old_col_names <- gsub("(\\S*?):.*", "\\1", old_meta_data)
    col_names <- gsub("(\\S*?):.*", "\\1", meta_data)
    if (!all(sort(col_names) == sort(old_col_names))) {
        stop(
            call. = FALSE,
            "old data has different variables, use override = TRUE"
        )
    }
    if (!all(sort(meta_data) == sort(old_meta_data))) {
        stop(
            call. = FALSE,
        "old data has different variable types or sorting, use override = TRUE"
        )
    }
    return(old_meta_data)
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
    file <- file.path(workdir(repo), file)
    file <- clean_data_path(file)

    if (!all(file.exists(file))) {
        stop("raw file and/or meta file missing")
    }
    meta_data <- readLines(file["meta_file"])
    meta_cols <- grep("^\\S*:$", meta_data)
    col_names <- gsub(":", "", meta_data[meta_cols])
    if (tail(meta_data, 1) == "optimized") {
        optimize <- TRUE
    } else if (tail(meta_data, 1) == "verbose") {
        optimize <- FALSE
    } else {
        stop("error in metadata")
    }
    raw_data <- read.table(
        file = file["raw_file"], header = !optimize,
        sep = "\t", quote = ifelse(optimize, "", "\"'"), dec = ".",
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
    if (optimize) {
        for (id in names(col_factor_level)) {
            raw_data[[id]] <- factor(
                raw_data[[id]],
                levels = seq_along(col_factor_level[[id]]),
                labels = col_factor_level[[id]]
            )
        }
    } else {
        for (id in names(col_factor_level)) {
            raw_data[[id]] <- factor(
                raw_data[[id]],
                levels = col_factor_level[[id]]
            )
        }
    }
    col_logical <- which(col_classes == "logical")
    for (id in col_logical) {
        raw_data[[id]] <- as.logical(raw_data[[id]])
    }


    col_posix <- which(col_classes == "POSIXct")
    for (id in col_posix) {
        raw_data[[id]] <- as.POSIXct(raw_data[[id]], origin = "1970-01-01")
    }

    col_date <- which(col_classes == "Date")
    for (id in col_date) {
        raw_data[[id]] <- as.Date(raw_data[[id]], origin = "1970-01-01")
    }

    return(raw_data)
}

##' optimize a vector for storage in to a git repository and add meta data
##' @param x the vector
##' @param optimize recode the data to get smaller text files. Defaults to TRUE
##' @details
##' \itemize{
##'    \item \code{meta.character} checks for the presence of \code{'NA'}.
##'    Because \code{\link{write_delim_git}} stores the data unquoted,
##'    \code{'NA'} and \code{NA} result in the same value in the file. Hence
##'    \code{\link{read_delim_git}} would report \code{'NA'} as \code{NA}.
##'    Therefore \code{meta.character} will throw an error with \code{'NA'} is
##'    detected.
##' }
##' @export
meta <- function(x, optimize = TRUE) {
    UseMethod("meta")
}

##' @export
meta.character <- function(x, optimize = TRUE) {
    attr(x, "meta") <- "    class: character"
    if (any(is.na(x))) {
        stop(
            call. = FALSE,
"The string 'NA' cannot be stored because it would be indistinguishable from the
missing value NA. Please replace or remove any 'NA' strings. Consider using a
factor."
        )
    }
    return(x)
}

##' @export
meta.integer <- function(x, optimize = TRUE) {
    attr(x, "meta") <- "    class: integer"
    return(x)
}

##' @export
meta.numeric <- function(x, optimize = TRUE) {
    attr(x, "meta") <- "    class: numeric"
    return(x)
}

##' @export
meta.factor <- function(x, optimize = TRUE) {
    if (optimize) {
        z <- as.integer(x)
    } else {
        z <- x
    }
    attr(z, "meta") <- paste(
        "    class: factor\n    levels:",
        paste("        -", levels(x), collapse = "\n"),
        sep = "\n"
    )
    return(z)
}

##' @export
meta.logical <- function(x, optimize = TRUE) {
    if (optimize) {
        x <- as.integer(x)
    }
    attr(x, "meta") <- "    class: logical"
    return(x)
}

##' @export
meta.complex <- function(x, optimize = TRUE) {
    attr(x, "meta") <- "    class: complex"
    return(x)
}

##' @export
meta.POSIXct <- function(x, optimize = TRUE) {
    if (optimize) {
        z <- unclass(x)
    } else {
        z <- x
    }
    attr(z, "meta") <- "    class: POSIXct\n    origin: 1970-01-01\n"
    return(z)
}

##' @export
meta.Date <- function(x, optimize = TRUE) {
    if (optimize) {
        z <- unclass(x)
    } else {
        z <- x
    }
    attr(z, "meta") <- "    class: Date\n    origin: 1970-01-01\n"
    return(z)
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

##' Remove data files
##' Remove all tsv and/or yml files within the path
##' @template repo-param
##' @param path the directory in which to clean all the data files
##' @param type which file type should be removed
##' @param recursive remove files in subdirectories too
##' @inheritParams write_delim_git
##' @export
rm_data <- function(
    repo = ".", path = NULL, type = c("tsv", "yml", "both"), recursive = TRUE,
    stage = FALSE
) {
    repo <- lookup_repository(repo)
    if (is.null(path) || !is.character(path))
        stop("'path' must be a character vector")
    if (length(path) != 1)
        stop("'path' must be a single value")
    type <- match.arg(type)

    local_path <- file.path(workdir(repo), path)
    if (type == "tsv") {
        to_do <- list.files(
            path = local_path,
            pattern = "\\.tsv$",
            recursive = recursive
        )
    } else if (type == "both") {
        to_do <- list.files(
            path = local_path,
            pattern = "\\.(tsv|yml)$",
            recursive = recursive
        )
    } else {
        to_do <- list.files(
            path = local_path,
            pattern = "\\.yml$",
            recursive = recursive
        )
        keep <- list.files(
            path = local_path,
            pattern = "\\.tsv$",
            recursive = recursive
        )
        keep <- gsub("\\.tsv$", ".yml", keep)
        to_do <- to_do[!to_do %in% keep]
    }
    rm_file(repo = repo, path = file.path(path, to_do))
    if (stage) {
        add(repo, path = to_do)
    }
}

##' Recent file change
##' Retrieve the most recent commit in which a file or data object was changed.
##' @template repo-param
##' @param path the path to the file or the data object. File extensions are silently ignored in case of a data object
##' @param data refers path to a file (FALSE) or a data object (TRUE). Defaults to FALSE
##' @export
##' @return A data.frame with commit, author and timestamp. Will contain multiple rows when multiple commits are made within the same second
recent_commit <- function(repo, path = NULL, data = FALSE) {
    repo <- lookup_repository(repo)
    if (is.null(path) || !is.character(path))
        stop("'path' must be a character vector")
    if (length(path) != 1)
        stop("'path' must be a single value")
    if (data) {
        path <- clean_data_path(path)
    }
    name <- basename(path)
    path <- unique(dirname(path))
message("path: ", path)
    if (path == ".") {
        path <- ""
    }
    blobs <- odb_blobs(repo)
    blobs <- blobs[blobs$path == path & blobs$name %in% name, ]
    blobs <- blobs[blobs$when == max(blobs$when), c("commit", "author", "when")]
    blobs <- unique(blobs)
    if (nrow(blobs) > 1) {
        warning("Multiple commits within the same second")
    }
    blobs
}
