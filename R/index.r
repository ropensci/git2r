## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2015 The git2r contributors
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
##' @rdname add-methods
##' @docType methods
##' @param repo The repository \code{object}.
##' @param path Character vector with file names or shell glob
##' patterns that will matched against files in the repository's
##' working directory. Each file that matches will be added to the
##' index (either updating an existing entry or adding a new entry).
##' @param ... Additional arguments to the method
##' @param force Add ignored files. Default is FALSE.
##' @return invisible(NULL)
##' @keywords methods
##' @include S4_classes.r
##' @examples
##' \dontrun{
##' ## Initialize a repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
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
##' ## Demonstrate glob expansion and that the current working
##' ## directory can be outside the repository's working directory.
##' setwd(tempdir())
##' dir.create(file.path(path, "glob_dir"))
##' writeLines("d", file.path(path, "glob_dir/d.txt"))
##' writeLines("e", file.path(path, "glob_dir/e.txt"))
##' writeLines("f", file.path(path, "glob_dir/f.txt"))
##' writeLines("g", file.path(path, "glob_dir/g.md"))
##' add(repo, "glob_dir/*txt")
##' status(repo)
##' }
setGeneric("add",
           signature = c("repo", "path"),
           function(repo, path, ...)
           standardGeneric("add"))

##' @rdname add-methods
##' @export
setMethod("add",
          signature(repo = "git_repository",
                    path = "character"),
          function(repo, path, force = FALSE)
          {
              ## Documentation for the pathspec argument in the
              ## libgit2 function 'git_index_add_all' that git2r use
              ## internally:
              ##
              ## The pathspec is a list of file names or shell glob
              ## patterns that will matched against files in the
              ## repository's working directory. Each file that
              ## matches will be added to the index (either updating
              ## an existing entry or adding a new entry).

              repo_wd <- normalizePath(workdir(repo), winslash = "/")
              if (!length(grep("/$", repo_wd)))
                  repo_wd <- paste0(repo_wd, "/")

              path <- sapply(path, function(p) {
                  np <- suppressWarnings(normalizePath(p, winslash = "/"))

                  ## Check if the path is a file, else let libgit2
                  ## handle this path unmodified.
                  if (!file.exists(np))
                      return(p)

                  ## Check if the file is in the repository's working
                  ## directory, else let libgit2 handle this path
                  ## unmodified.
                  if (!length(grep(paste0("^", repo_wd), np)))
                      return(p)

                  ## Change the path to be relative to the repository's
                  ## working directory. Substitute common prefix with ""
                  sub(paste0("^", repo_wd), "", np)
              })

              .Call(git2r_index_add_all, repo, path, force)

              invisible(NULL)
          }
)

##' Remove files from the working tree and from the index
##'
##' @rdname rm_file-methods
##' @docType methods
##' @param repo The repository \code{object}.
##' @param path character vector with filenames to remove. The path
##' must be relative to the repository's working folder. Only files
##' known to Git are removed.
##' @return invisible(NULL)
##' @keywords methods
##' @include S4_classes.r
##' @examples
##' \dontrun{
##' ## Initialize a repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
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
##'
setGeneric("rm_file",
           signature = c("repo", "path"),
           function(repo, path)
           standardGeneric("rm_file"))

##' @rdname rm_file-methods
##' @export
setMethod("rm_file",
          signature(repo = "git_repository",
                    path = "character"),
          function(repo, path)
          {
              if (length(path)) {
                  ## Check that files exists and are known to Git
                  if (!all(file.exists(paste0(workdir(repo), path)))) {
                      stop(sprintf("pathspec '%s' did not match any files. ",
                                   path[!file.exists(paste0(workdir(repo), path))]))
                  }

                  if (any(file.info(paste0(workdir(repo), path))$isdir)) {
                      stop(sprintf("pathspec '%s' did not match any files. ",
                                   path[exists(paste0(workdir(repo), path))]))
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
                      file.remove(paste0(workdir(repo), x))
                      .Call(git2r_index_remove_bypath, repo, x)
                  })
              }

              invisible(NULL)
          }
)
