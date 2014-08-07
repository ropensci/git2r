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

##' Class \code{"git_blame"}
##'
##' @title  S4 class to handle a git blame for a single file
##' @section Slots:
##' \describe{
##'   \item{path}{
##'     The path to the file of the blame
##'   }
##'   \item{hunks}{
##'     List of blame hunks
##'   }
##'   \item{repo}{
##'     The S4 class git_repository that contains the file
##'   }
##' }
##' @rdname git_blame-class
##' @docType class
##' @keywords classes
##' @keywords methods
##' @include repository.r
##' @export
##' @examples \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Get blame for file in repository
##' blame(repo, ".gitignore")
##' }
setClass("git_blame",
         slots=c(path  = "character",
                 hunks = "list",
                 repo  = "git_repository"),
         validity=function(object) {
             errors <- character()

             if (length(errors) == 0) TRUE else errors
         }
)

##' Class \code{"git_blame_hunk"}
##'
##' @title  S4 class to handle a blame hunk
##' @section Slots:
##' \describe{
##'   \item{lines_in_hunk}{
##'     The number of lines in this hunk
##'   }
##'   \item{final_commit_id}{
##'     The hex of the commit where this line was last changed
##'   }
##'   \item{final_start_line_number}{
##'     The 1-based line number where this hunk begins, in the final
##'     version of the file
##'   }
##'   \item{final_signature}{
##'     :TODO:DOCUMENTATION:
##'   }
##'   \item{orig_commit_id}{
##'     :TODO:DOCUMENTATION:
##'   }
##'   \item{orig_start_line_number}{
##'     :TODO:DOCUMENTATION:
##'   }
##'   \item{orig_signature}{
##'     :TODO:DOCUMENTATION:
##'   }
##'   \item{orig_path}{
##'     :TODO:DOCUMENTATION:
##'   }
##'   \item{boundary}{
##'     :TODO:DOCUMENTATION:
##'   }
##'   \item{repo}{
##'     The S4 class git_repository that contains the blame hunk
##'   }
##' }
##' @rdname git_blame_hunk-class
##' @docType class
##' @keywords classes
##' @keywords methods
##' @include repository.r
##' @include signature.r
##' @export
setClass("git_blame_hunk",
         slots=c(lines_in_hunk           = "integer",
                 final_commit_id         = "character",
                 final_start_line_number = "integer",
                 final_signature         = "git_signature",
                 orig_commit_id          = "character",
                 orig_start_line_number  = "integer",
                 orig_signature          = "git_signature",
                 orig_path               = "character",
                 boundary                = "logical",
                 repo                    = "git_repository"),
         validity=function(object) {
             errors <- character()

             if (length(errors) == 0) TRUE else errors
         }
)

##' Get blame for file
##'
##' @rdname blame-methods
##' @docType methods
##' @param repo The repository
##' @param path Path to the file to consider
##' @return S4 class git_blame object
##' @keywords methods
setGeneric("blame",
           signature = c("repo", "path"),
           function(repo,
                    path)
           standardGeneric("blame"))

##' @rdname blame-methods
##' @export
setMethod("blame",
          signature(repo = "git_repository",
                    path = "character"),
          function(repo,
                   path)
          {
              .Call(git2r_blame_file, repo, path)
          }
)
