## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2023 The git2r contributors
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

##' Add file(s) to index
##'
##' @template repo-param
##' @param path Character vector with file names or shell glob
##'     patterns that will matched against files in the repository's
##'     working directory. Each file that matches will be added to the
##'     index (either updating an existing entry or adding a new
##'     entry).
##' @param force Add ignored files. Default is FALSE.
##' @return invisible(NULL)
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Create a file
##' writeLines("a", file.path(path, "a.txt"))
##'
##' ## Add file to repository and view status
##' add(repo, "a.txt")
##' status(repo)
##'
##' ## Add file with a leading './' when the repository working
##' ## directory is the current working directory
##' setwd(path)
##' writeLines("b", file.path(path, "b.txt"))
##' add(repo, "./b.txt")
##' status(repo)
##'
##' ## Add a file in a sub-folder with sub-folder as the working
##' ## directory. Create a file in the root of the repository
##' ## working directory that will remain untracked.
##' dir.create(file.path(path, "sub_dir"))
##' setwd("./sub_dir")
##' writeLines("c", file.path(path, "c.txt"))
##' writeLines("c", file.path(path, "sub_dir/c.txt"))
##' add(repo, "c.txt")
##' status(repo)
##'
##' ## Add files with glob expansion when the current working
##' ## directory is outside the repository's working directory.
##' setwd(tempdir())
##' dir.create(file.path(path, "glob_dir"))
##' writeLines("d", file.path(path, "glob_dir/d.txt"))
##' writeLines("e", file.path(path, "glob_dir/e.txt"))
##' writeLines("f", file.path(path, "glob_dir/f.txt"))
##' writeLines("g", file.path(path, "glob_dir/g.md"))
##' add(repo, "glob_dir/*txt")
##' status(repo)
##'
##' ## Add file with glob expansion with a relative path when
##' ## the current working directory is inside the repository's
##' ## working directory.
##' setwd(path)
##' add(repo, "./glob_dir/*md")
##' status(repo)
##' }
add <- function(repo = ".", path = NULL, force = FALSE) {
    ## Documentation for the pathspec argument in the libgit2 function
    ## 'git_index_add_all' that git2r use internally:
    ##
    ## The pathspec is a list of file names or shell glob patterns
    ## that will matched against files in the repository's working
    ## directory. Each file that matches will be added to the index
    ## (either updating an existing entry or adding a new entry).

    if (!is.character(path))
        stop("'path' must be a character vector")

    repo <- lookup_repository(repo)
    repo_wd <- normalizePath(workdir(repo), winslash = "/")
    path <- vapply(path, sanitize_path, character(1), repo_wd = repo_wd)

    .Call(git2r_index_add_all, repo, path, isTRUE(force))

    invisible(NULL)
}

sanitize_path <- function(p, repo_wd) {
    np <- suppressWarnings(normalizePath(p, winslash = "/"))

    if (!length(grep("/$", repo_wd)))
        repo_wd <- paste0(repo_wd, "/")

    ## Check if the normalized path is a non-file e.g. a glob.
    if (!file.exists(np)) {
        ## Check if the normalized path starts with a leading './'
        if (length(grep("^[.]/", np))) {
            nd <- suppressWarnings(normalizePath(dirname(p), winslash = "/"))
            if (!length(grep("/$", nd)))
                nd <- paste0(nd, "/")
            np <- paste0(nd, basename(np))
        }
    }

    ## Check if the file is in the repository's working directory,
    ## else let libgit2 handle this path unmodified.
    if (!length(grep(paste0("^", repo_wd), np)))
        return(p)

    ## Change the path to be relative to the repository's working
    ## directory. Substitute common prefix with ""
    sub(paste0("^", repo_wd), "", np)
}

##' Remove files from the working tree and from the index
##'
##' @template repo-param
##' @param path character vector with filenames to remove. Only files
##'     known to Git are removed.
##' @return invisible(NULL)
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Create a file
##' writeLines("Hello world!", file.path(path, "file-to-remove.txt"))
##'
##' ## Add file to repository
##' add(repo, "file-to-remove.txt")
##' commit(repo, "First commit message")
##'
##' ## Remove file
##' rm_file(repo, "file-to-remove.txt")
##'
##' ## View status of repository
##' status(repo)
##' }
rm_file <- function(repo = ".", path = NULL) {
    if (!is.character(path))
        stop("'path' must be a character vector")

    repo <- lookup_repository(repo)

    if (length(path)) {
        repo_wd <- workdir(repo)
        repo_wd <- normalizePath(workdir(repo), winslash = "/")
        path <- vapply(path, sanitize_path, character(1), repo_wd = repo_wd)

        ## Check that files exists and are known to Git
        if (!all(file.exists(file.path(repo_wd, path)))) {
            stop(sprintf("pathspec '%s' did not match any files. ",
                         path[!file.exists(file.path(repo_wd, path))]))
        }

        if (any(file.info(file.path(repo_wd, path))$isdir)) {
            stop(sprintf("pathspec '%s' did not match any files. ",
                         path[exists(file.path(repo_wd, path))]))
        }

        s <- status(repo, staged = TRUE, unstaged = TRUE,
                    untracked = TRUE, ignored = TRUE)
        if (any(path %in% c(s$ignored, s$untracked))) {
            stop(sprintf("pathspec '%s' did not match any files. ",
                         path[path %in% c(s$ignored, s$untracked)]))
        }

        if (any(path %in% s$staged)) {
            stop(sprintf("'%s' has changes staged in the index. ",
                         path[path %in% s$staged]))
        }

        if (any(path %in% s$unstaged)) {
            stop(sprintf("'%s' has local modifications. ",
                         path[path %in% s$unstaged]))
        }

        ## Remove and stage files
        lapply(path, function(x) {
            file.remove(file.path(repo_wd, x))
            .Call(git2r_index_remove_bypath, repo, x)
        })
    }

    invisible(NULL)
}

##' Remove an index entry corresponding to a file on disk
##'
##' @template repo-param
##' @param path character vector with filenames to remove. The path
##'     must be relative to the repository's working folder. It may
##'     exist. If this file currently is the result of a merge
##'     conflict, this file will no longer be marked as
##'     conflicting. The data about the conflict will be moved to the
##'     "resolve undo" (REUC) section.
##' @return invisible(NULL)
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##'
##' ## Create a file
##' writeLines("Hello world!", file.path(path, "file-to-remove.txt"))
##'
##' ## Add file to repository
##' add(repo, "file-to-remove.txt")
##'
##' ## View status of repository
##' status(repo)
##'
##' ## Remove file
##' index_remove_bypath(repo, "file-to-remove.txt")
##'
##' ## View status of repository
##' status(repo)
##' }
index_remove_bypath <- function(repo = ".", path = NULL) {
    .Call(git2r_index_remove_bypath, lookup_repository(repo), path)
    invisible(NULL)
}
