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

##' Generate list of contributors for git2r and libgit2
##'
##' @return person list
git2r_libgit2_contributors <- function() {
    git2r <- c("Stefan Widgren (git2r contributor)",
               "Karthik Ram (git2r contributor)",
               "Gabor Csardi (git2r contributor)",
               "Gregory Jefferis (git2r contributor)",
               "Thomas Rosendal (git2r contributor)")

    ## Add libgit2 authors
    libgit2 <- readLines("scripts/AUTHORS_libgit2")
    libgit2 <- libgit2[-(1:3)]
    libgit2 <- paste0(libgit2, " (libgit2 contributor)")

    c(git2r, libgit2)
}

##' Build DESCRIPTION
##'
##' @return invisible(NULL)
build_DESCRIPTION <- function() {
    version <- "0.1"
    date <- "2014-09-09"

    lines <- c(
        "Package: git2r",
        "Title: Programmatic Access to Git Repositories from R",
        "Description: git2r is a package for accessing Git repositories from R",
        "    using the libgit2 library. The package enables running some basic",
        "    git commands from R and to extract data from a Git repository.")

    lines <- c(lines, sprintf("Version: %s", version))
    lines <- c(lines, sprintf("Date: %s", date))

    lines <- c(
        lines,
        "License: GPL-2",
        "Copyright: The package includes the source code of libgit2,",
        "    (https://github.com/libgit2/libgit2) See the included NOTICE file",
        "    for the libgit2 license text. The libgit2 printf calls in cache.c",
        "    and util.c have been changed to use the R printing routine",
        "    Rprintf.",
        "URL: https://github.com/ropensci/git2r",
        "BugReports: https://github.com/ropensci/git2r/issues",
        "Maintainer: Stefan Widgren <stefan.widgren@gmail.com>")

    ctbs <- paste0("    ", git2r_libgit2_contributors())
    lines <- c(lines, "Author:", ctbs)

    lines <- c(
        lines,
        "Depends:",
        "    R (>= 3.0.2),",
        "    methods",
        "Type: Package",
        "Encoding: UTF-8",
        "LazyData: true",
        "Biarch: true",
        "NeedsCompilation: yes",
        "SystemRequirements: While the package provides git functionality without the",
        "    need for dependencies, it can make use of a few libraries to add to it:",
        "     - OpenSSL (non-Windows) to talk over HTTPS and provide the SHA-1 functions",
        "     - LibSSH2 to enable the SSH transport",
        "     - iconv (OSX) to handle the HFS+ path encoding peculiarities")

    writeLines(lines, "DESCRIPTION")

    invisible(NULL)
}

build_DESCRIPTION()
