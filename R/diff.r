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


#' Git diff
#'
#' @name git_diff-class
#' @docType class
#' @keywords classes
#' @include repository.r
#' @export
setClass("git_diff",
         slots=c(old   = "ANY",
                 new   = "ANY",
                 files = "list"),
         prototype=list(old=NA_character_,
                        new=NA_character_))

#' Git diff file
#'
#' @name git_diff_file-class
#' @docType class
#' @keywords classes
#' @export
setClass("git_diff_file",
         slots=c(old_file = "character",
                 new_file = "character",
                 hunks = "list"))

#' Git diff hunk
#'
#' @name git_diff_hunk-class
#' @docType class
#' @keywords classes
#' @export
setClass("git_diff_hunk",
         slots=c(old_start = "integer",
                 old_lines = "integer",
                 new_start = "integer",
                 new_lines = "integer",
                 header    = "character",
                 lines     = "list"))

#' Git diff line
#'
#' @name git_diff_line-class
#' @docType class
#' @keywords classes
#' @export
setClass("git_diff_line",
         slots=c(origin      = "integer",
                 old_lineno  = "integer",
                 new_lineno  = "integer",
                 num_lines   = "integer",
                 content     = "character"))

#' @export
setMethod("length",
          signature(x = "git_diff"),
          function(x)
          {
              length(x@files)
          }
)

#' Show a diff
#'
#' @aliases show,git_diff-methods
#' @docType methods
#' @param object The diff \code{object}.
#' @keywords methods
#' @export
setMethod("show",
          signature(object = "git_diff"),
          function(object)
          {
              cat("Old:  ")
              if (is.character(object@old)) {
                  cat(object@old, "\n")
              } else if (is(object@old, "git_tree")) {
                  show(object@old)
              } else {
                  cat("\n")
                  show(object@old)
              }
              cat("New:  ")
              if (is.character(object@new)) {
                  cat(object@new, "\n")
              } else if (is(object@new, "git_tree")) {
                  show(object@new)
              } else {
                  cat("\n")
                  show(object@new)
              }
          }
)

lines_per_file <- function(diff) {
    lapply(diff@files, function(x) {
        del <- add <- 0
        for (h in x@hunks) {
            for (l in h@lines) {
                if (l@origin == 45) {
                    del <- del + l@num_lines
                } else if (l@origin == 43) {
                    add <- add + l@num_lines
                }
            }
        }
        list(file=x@new_file, del=del, add=add)
    })
}

print_lines_per_file <- function(diff) {
  lpf <- lines_per_file(diff)
  files <- sapply(lpf, function(x) x$file)
  del <- sapply(lpf, function(x) x$del)
  add <- sapply(lpf, function(x) x$add)
  paste0(format(files), " | ", "-", format(del), " +", format(add))
}

hunks_per_file <- function(diff) {
    sapply(diff@files, function(x) length(x@hunks))
}

#' Show the summary of a diff
#'
#' @aliases summary,git_diff-methods
#' @docType methods
#' @param object The diff \code{object}.
#' @keywords methods
#' @export
setMethod("summary",
          signature(object = "git_diff"),
          function(object, ...)
          {
              show(object)
              if (length(object) > 0) {
                  plpf <- print_lines_per_file(object)
                  hpf <- hunks_per_file(object)
                  hunk_txt <- ifelse(hpf > 1, " hunks",
                                     ifelse(hpf > 0, " hunk",
                                            " hunk (binary file)"))
                  phpf <- paste0("  in ", format(hpf), hunk_txt)
                  cat("Summary:", paste0(plpf, phpf), sep="\n")
              } else {
                  cat("No changes.\n")
              }
          }
)

#' Changes between commits, trees, working tree, etc.
#'
#' @rdname diff-methods
#' @docType methods
#' @return A git_diff object
#' @keywords methods
setGeneric("diff",
           signature = c("object"),
           function(object, ...)
           standardGeneric("diff"))

#' @rdname diff-methods
#' @param object A git_repository object
#' @param index Flag, whether to compare the index to HEAD. If
#'        FALSE (the default), then the working tree is compared to
#'        the index.
#' @export
setMethod("diff",
          signature(object = "git_repository"),
          function(object, index=FALSE)
          {
              .Call("git2r_diff", repo = object, tree1 = NULL,
                    tree2 = NULL, index = index)
          }
)

#' @rdname diff-methods
#' @param object The old git_tree object to compare to
#' @param new_tree The new git_tree object to compare, or NULL.
#'        If NULL, then we use the working directory or the index (see
#'        the \code{index} argument).
#' @param index Whether to use the working directory (by default),
#'        or the index (if set to TRUE) in the comparison to
#'        \code{object}.
#' @export
setMethod("diff",
          signature(object = "git_tree"),
          function(object, new_tree=NULL, index=FALSE)
          {
              if (!is.null(new_tree)) {
                  if (! is(new_tree, "git_tree")) {
                      stop("Not a git tree")
                  }
                  if (object@repo@path != new_tree@repo@path) {
                      stop("Cannot compare trees in different repositories")
                  }
              }
              .Call("git2r_diff", repo = NULL, tree1 = object,
                    tree2 = new_tree, index = index)
          }
)
