## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2022 The git2r contributors
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

##' Number of files in git_diff object
##'
##' @param x The git_diff \code{object}
##' @return a non-negative integer
##' @export
length.git_diff <- function(x) {
    length(x$files)
}

##' @export
print.git_diff <- function(x, ...) {
    cat("Old:  ")
    if (is.character(x$old)) {
        cat(x$old, "\n")
    } else if (inherits(x$old, "git_tree")) {
        print(x$old)
    } else {
        cat("\n")
        print(x$old)
    }

    cat("New:  ")
    if (is.character(x$new)) {
        cat(x$new, "\n")
    } else if (inherits(x$new, "git_tree")) {
        print(x$new)
    } else {
        cat("\n")
        print(x$new)
    }

    invisible(x)
}

lines_per_file <- function(diff) {
    lapply(diff$files, function(x) {
        del <- add <- 0
        for (h in x$hunks) {
            for (l in h$lines) {
                if (l$origin == 45) {
                    del <- del + l$num_lines
                } else if (l$origin == 43) {
                    add <- add + l$num_lines
                }
            }
        }
        list(file = x$new_file, del = del, add = add)
    })
}

print_lines_per_file <- function(diff) {
  lpf <- lines_per_file(diff)
  files <- vapply(lpf, function(x) x$file, character(1))
  del <- vapply(lpf, function(x) x$del, numeric(1))
  add <- vapply(lpf, function(x) x$add, numeric(1))
  paste0(format(files), " | ", "-", format(del), " +", format(add))
}

hunks_per_file <- function(diff) {
    vapply(diff$files, function(x) length(x$hunks), numeric(1))
}

##' @export
summary.git_diff <- function(object, ...) {
    print(object)
    if (length(object) > 0) {
        plpf <- print_lines_per_file(object)
        hpf <- hunks_per_file(object)
        hunk_txt <- ifelse(hpf > 1, " hunks",
                    ifelse(hpf > 0, " hunk",
                           " hunk (binary file)"))
        phpf <- paste0("  in ", format(hpf), hunk_txt)
        cat("Summary:", paste0(plpf, phpf), sep = "\n")
    } else {
        cat("No changes.\n")
    }
}

##' Changes between commits, trees, working tree, etc.
##'
##' @rdname diff-methods
##' @export
##' @param x A \code{git_repository} object or the old \code{git_tree}
##'     object to compare to.
##' @param index \describe{
##'   \item{\emph{When object equals a git_repository}}{
##'     Whether to compare the index to HEAD. If FALSE (the default),
##'     then the working tree is compared to the index.
##'   }
##'   \item{\emph{When object equals a git_tree}}{
##'     Whether to use the working directory (by default), or the index
##'     (if set to TRUE) in the comparison to \code{object}.
##'   }
##' }
##' @param as_char logical: should the result be converted to a
##'     character string?. Default is FALSE.
##' @param filename If as_char is TRUE, then the diff can be written
##'     to a file with name filename (the file is overwritten if it
##'     exists). Default is NULL.
##' @param context_lines The number of unchanged lines that define the
##'     boundary of a hunk (and to display before and after). Defaults
##'     to 3.
##' @param interhunk_lines The maximum number of unchanged lines
##'     between hunk boundaries before the hunks will be merged into
##'     one. Defaults to 0.
##' @param old_prefix The virtual "directory" prefix for old file
##'     names in hunk headers. Default is "a".
##' @param new_prefix The virtual "directory" prefix for new file
##'     names in hunk headers. Defaults to "b".
##' @param id_abbrev The abbreviation length to use when formatting
##'     object ids. Defaults to the value of 'core.abbrev' from the
##'     config, or 7 if NULL.
##' @param path A character vector of paths / fnmatch patterns to
##'     constrain diff. Default is NULL which include all paths.
##' @param max_size A size (in bytes) above which a blob will be
##'     marked as binary automatically; pass a negative value to
##'     disable. Defaults to 512MB when max_size is NULL.
##' @return A \code{git_diff} object if as_char is FALSE. If as_char
##'     is TRUE and filename is NULL, a character string, else NULL.
##' @section Line endings:
##'
##' Different operating systems handle line endings
##' differently. Windows uses both a carriage-return character and a
##' linefeed character to represent a newline in a file. While Linux
##' and macOS use only the linefeed character for a newline in a
##' file. To avoid problems in your diffs, you can configure Git to
##' properly handle line endings using the \verb{core.autocrlf}
##' setting in the Git config file, see the Git documentation
##' (\url{https://git-scm.com/}).
##' @examples
##' \dontrun{
##' ## Initialize a repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Config user
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Create a file, add, commit
##' writeLines("Hello world!", file.path(path, "test.txt"))
##' add(repo, "test.txt")
##' commit(repo, "Commit message")
##'
##' ## Change the file
##' writeLines(c("Hello again!", "Here is a second line", "And a third"),
##'            file.path(path, "test.txt"))
##'
##' ## diff between index and workdir
##' diff_1 <- diff(repo)
##' summary(diff_1)
##' cat(diff(repo, as_char=TRUE))
##'
##' ## Diff between index and HEAD is empty
##' diff_2 <- diff(repo, index=TRUE)
##' summary(diff_2)
##' cat(diff(repo, index=TRUE, as_char=TRUE))
##'
##' ## Diff between tree and working dir, same as diff_1
##' diff_3 <- diff(tree(commits(repo)[[1]]))
##' summary(diff_3)
##' cat(diff(tree(commits(repo)[[1]]), as_char=TRUE))
##'
##' ## Add changes, diff between index and HEAD is the same as diff_1
##' add(repo, "test.txt")
##' diff_4 <- diff(repo, index=TRUE)
##' summary(diff_4)
##' cat(diff(repo, index=TRUE, as_char=TRUE))
##'
##' ## Diff between tree and index
##' diff_5 <- diff(tree(commits(repo)[[1]]), index=TRUE)
##' summary(diff_5)
##' cat(diff(tree(commits(repo)[[1]]), index=TRUE, as_char=TRUE))
##'
##' ## Diff between two trees
##' commit(repo, "Second commit")
##' tree_1 <- tree(commits(repo)[[2]])
##' tree_2 <- tree(commits(repo)[[1]])
##' diff_6 <- diff(tree_1, tree_2)
##' summary(diff_6)
##' cat(diff(tree_1, tree_2, as_char=TRUE))
##'
##' ## Binary files
##' set.seed(42)
##' writeBin(as.raw((sample(0:255, 1000, replace=TRUE))),
##'          con=file.path(path, "test.bin"))
##' add(repo, "test.bin")
##' diff_7 <- diff(repo, index=TRUE)
##' summary(diff_7)
##' cat(diff(repo, index=TRUE, as_char=TRUE))
##' }
diff.git_repository <- function(x,
                                index    = FALSE,
                                as_char  = FALSE,
                                filename = NULL,
                                context_lines = 3,
                                interhunk_lines = 0,
                                old_prefix = "a",
                                new_prefix = "b",
                                id_abbrev = NULL,
                                path = NULL,
                                max_size = NULL,
                                ...) {
    if (isTRUE(as_char)) {
        ## Make sure filename is character(0) to write to a
        ## character vector or a character vector with path in
        ## order to write to a file.
        filename <- as.character(filename)
        if (any(identical(filename, NA_character_),
                identical(nchar(filename), 0L))) {
            filename <- character(0)
        } else if (length(filename)) {
            filename <- normalizePath(filename, mustWork = FALSE)
        }
    } else {
        ## Make sure filename is NULL
        filename <- NULL
    }

    if (!is.null(id_abbrev))
        id_abbrev <- as.integer(id_abbrev)

    if (!is.null(max_size))
        max_size <- as.integer(max_size)

    .Call(git2r_diff, x, NULL, NULL, index, filename,
          as.integer(context_lines), as.integer(interhunk_lines),
          old_prefix, new_prefix, id_abbrev, path, max_size)
}

##' @rdname diff-methods
##' @param new_tree The new git_tree object to compare, or NULL.  If
##'     NULL, then we use the working directory or the index (see the
##'     \code{index} argument).
##' @param ... Not used.
##' @export
diff.git_tree <- function(x,
                          new_tree = NULL,
                          index    = FALSE,
                          as_char  = FALSE,
                          filename = NULL,
                          context_lines = 3,
                          interhunk_lines = 0,
                          old_prefix = "a",
                          new_prefix = "b",
                          id_abbrev = NULL,
                          path = NULL,
                          max_size = NULL,
                          ...) {
    if (isTRUE(as_char)) {
        ## Make sure filename is character(0) to write to a character
        ## vector or a character vector with path in order to write to
        ## a file.
        filename <- as.character(filename)
        if (any(identical(filename, NA_character_),
                identical(nchar(filename), 0L))) {
            filename <- character(0)
        } else if (length(filename)) {
            filename <- normalizePath(filename, mustWork = FALSE)
        }
    } else {
        ## Make sure filename is NULL
        filename <- NULL
    }

    if (!is.null(new_tree)) {
        if (!inherits(new_tree, "git_tree")) {
            stop("Not a git tree")
        }
        if (x$repo$path != new_tree$repo$path) {
            stop("Cannot compare trees in different repositories")
        }
    }

    if (!is.null(id_abbrev))
        id_abbrev <- as.integer(id_abbrev)

    if (!is.null(max_size))
        max_size <- as.integer(max_size)

    .Call(git2r_diff, NULL, x, new_tree, index, filename,
          as.integer(context_lines), as.integer(interhunk_lines),
          old_prefix, new_prefix, id_abbrev, path, max_size)
}

##' @export
base::diff
