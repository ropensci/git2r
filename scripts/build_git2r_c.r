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

##' Extract .NAME in .Call(.NAME
##'
##' @param files R files to extract .NAME from
##' @return data.frame with columns filename and .NAME
extract_git2r_calls <- function(files) {
    df <- lapply(files, function(filename) {
        ## Read file
        lines <- readLines(file.path("R", filename))

        ## Trim comments
        comments <- gregexpr("#", lines)
        for (i in seq_len(length(comments))) {
            start <- as.integer(comments[[i]])
            if (start[1] > 0) {
                if (start[1] > 1) {
                    lines[i] <- substr(lines[i], 1, start[1])
                } else {
                    lines[i] <- ""
                }
            }
        }

        ## Trim whitespace
        lines <- sub("^\\s*", "", sub("\\s*$", "", lines))

        ## Collapse to one line
        lines <- paste0(lines, collapse=" ")

        ## Find .Call
        pattern <- "[.]Call[[:space:]]*[(][[:space:]]*[.[:alpha:]\"][^\\),]*"
        calls <- gregexpr(pattern, lines)
        start <- as.integer(calls[[1]])

        if (start[1] > 0) {
            ## Extract .Call
            len <- attr(calls[[1]], "match.length")
            calls <- substr(rep(lines, length(start)), start, start + len - 1)

            ## Trim .Call to extract .NAME
            pattern <- "[.]Call[[:space:]]*[(][[:space:]]*"
            calls <- sub(pattern, "", calls)
            return(data.frame(filename = filename,
                              .NAME = calls,
                              stringsAsFactors = FALSE))
        }

        return(NULL)
    })

    df <- do.call("rbind", df)
    df[order(df$filename),]
}

##' Check that .NAME in .Call(.NAME is prefixed with 'git2r_'
##'
##' Raise an error in case of missing 'git2r_' prefix
##' @param calls data.frame with the name of the C function to call
##' @return invisible NULL
check_git2r_prefix <- function(calls) {
    .NAME <- grep("git2r_", calls$.NAME, value=TRUE, invert=TRUE)

    if (!identical(length(.NAME), 0L)) {
        i <- which(calls$.NAME == .NAME)
        msg <- sprintf("%s in %s\n", calls$.NAME[i], calls$filename[i])
        msg <- c("\n\nMissing 'git2r_' prefix:\n", msg, "\n")
        stop(msg)
    }

    invisible(NULL)
}

##' Check that .NAME is a registered symbol in .Call(.NAME
##'
##' Raise an error in case of .NAME is of the form "git2r_"
##' @param calls data.frame with the name of the C function to call
##' @return invisible NULL
check_git2r_use_registered_symbol <- function(calls) {
    .NAME <- grep("^\"", calls$.NAME)

    if (!identical(length(.NAME), 0L)) {
        msg <- sprintf("%s in %s\n", calls$.NAME[.NAME], calls$filename[.NAME])
        msg <- c("\n\nUse registered symbol instead of:\n", msg, "\n")
        stop(msg)
    }

    invisible(NULL)
}

## Check that all git2r C functions are prefixed with 'git2r_' and
## registered
calls <- extract_git2r_calls(list.files("R", "*.r"))
check_git2r_prefix(calls)
check_git2r_use_registered_symbol(calls)
