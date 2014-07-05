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

##' Class \code{"git_commit"}
##'
##' @title S4 class to handle a git commit.
##' @section Slots:
##' \describe{
##'   \item{hex}{
##'     40 char hexadecimal string
##'   }
##'   \item{author}{
##'     An author signature
##'   }
##'   \item{committer}{
##'     The committer signature
##'   }
##'   \item{summary}{
##'     The short "summary" of a git commit message, comprising the first
##'     paragraph of the message with whitespace trimmed and squashed.
##'   }
##'   \item{message}{
##'     The message of a commit
##'   }
##'   \item{repo}{
##'     The S4 class git_repository that contains the commit
##'   }
##' }
##' @name git_commit-class
##' @docType class
##' @keywords classes
##' @section Methods:
##' \describe{
##'   \item{show}{\code{signature(object = "git_commit")}}
##' }
##' @keywords methods
##' @include diff.r
##' @include repository.r
##' @include signature.r
##' @export
setClass("git_commit",
         slots=c(hex       = "character",
                 author    = "git_signature",
                 committer = "git_signature",
                 summary   = "character",
                 message   = "character",
                 repo      = "git_repository"),
         prototype=list(summary=NA_character_,
                        message=NA_character_))

##' Ahead Behind
##'
##' Count the number of unique commits between two commit objects.
##' @rdname ahead_behind-methods
##' @docType methods
##' @param local a S4 class git_commit \code{object}.
##' @param upstream a S4 class git_commit \code{object}.
##' @return An integer vector of length 2 with number of commits that
##' the upstream commit is ahead and behind the local commit
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ahead_behind(commits(repo)[[1]], commits(repo)[[2]])
##' }
##'
setGeneric("ahead_behind",
           signature = c("local", "upstream"),
           function(local, upstream)
           standardGeneric("ahead_behind"))

##' @rdname ahead_behind-methods
##' @export
setMethod("ahead_behind",
          signature(local = "git_commit", upstream = "git_commit"),
          function (local, upstream)
          {
              stopifnot(identical(local@repo, upstream@repo))
              .Call("git2r_graph_ahead_behind", local, upstream)
          }
)

##' Commits
##'
##' @rdname commits-methods
##' @docType methods
##' @param repo The repository \code{object}.
##' @param topological Sort the commits in topological order (parents
##' before children); can be combined with time sorting. Default is
##' TRUE.
##' @param time Sort the commits by commit time; Can be combined with
##' topological sorting. Default is TRUE.
##' @param reverse Sort the commits in reverse order; can be combined
##' with topological and/or time sorting. Default is FALSE.
##' @return list of commits in repository
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## List commits in repository
##' commits(repo)
##'
##' ## List commits direct from path
##' commits("path/to/git2r")
##' }
##'
setGeneric("commits",
           signature = "repo",
           function(repo,
                    topological = TRUE,
                    time        = TRUE,
                    reverse     = FALSE)
           standardGeneric("commits"))

##' @rdname commits-methods
##' @export
setMethod("commits",
          signature(repo = "character"),
          function(repo, topological, time, reverse)
          {
              commits(repository(repo),
                      topological = topological,
                      time = time,
                      reverse = reverse)
          }
)

##' @rdname commits-methods
##' @include repository.r
##' @export
setMethod("commits",
          signature(repo = "git_repository"),
          function(repo, topological, time, reverse)
          {
              .Call("git2r_revwalk_list",
                    repo,
                    topological,
                    time,
                    reverse)
          }
)

##' Descendant
##'
##' Determine if a commit is the descendant of another commit
##' @rdname descendant_of-methods
##' @docType methods
##' @param commit a S4 class git_commit \code{object}.
##' @param ancestor a S4 class git_commit \code{object} to check if
##' ancestor to \code{commit}.
##' @return TRUE if commit is descendant of \code{ancestor}, else
##' FALSE
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' descendant_of(commits(repo)[[1]], commits(repo)[[2]])
##' }
##'
setGeneric("descendant_of",
           signature = c("commit", "ancestor"),
           function(commit, ancestor)
           standardGeneric("descendant_of"))

##' @rdname descendant_of-methods
##' @export
setMethod("descendant_of",
          signature(commit = "git_commit", ancestor = "git_commit"),
          function (commit, ancestor)
          {
              stopifnot(identical(commit@repo, ancestor@repo))
              .Call("git2r_graph_descendant_of", commit, ancestor)
          }
)

##' Check if object is S4 class git_commit
##'
##' @param object Check if object is S4 class git_commit
##' @return TRUE if object is S4 class git_commit, else FALSE
##' @keywords methods
##' @export
is_commit <- function(object) {
    is(object = object, class2 = "git_commit")
}

##' Is merge
##'
##' Determine if a commit is a merge commit, i.e. has more than one
##' parent.
##' @rdname is_merge-methods
##' @docType methods
##' @param commit a S4 class git_commit \code{object}.
##' @return TRUE if commit has more than one parent, else FALSE
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Display list of merge commits in repository
##' invisible(lapply(commits(repo)[sapply(commits(repo), is_merge)], show))
##' }
##'
setGeneric("is_merge",
           signature = c("commit"),
           function(commit)
           standardGeneric("is_merge"))

##' @rdname is_merge-methods
##' @export
setMethod("is_merge",
          signature(commit = "git_commit"),
          function (commit)
          {
              length(parents(commit)) > 1
          }
)

##' Parents
##'
##' Get parents of a commit.
##' @rdname parents-methods
##' @docType methods
##' @param object a S4 class git_commit \code{object}.
##' @return list of S4 git_commit objects
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' parents(commits(repo)[[1]])
##' }
##'
setGeneric("parents",
           signature = "object",
           function(object)
           standardGeneric("parents"))

##' @rdname parents-methods
##' @export
setMethod("parents",
          signature(object = "git_commit"),
          function(object)
          {
              .Call("git2r_commit_parent_list", object)
          }
)

##' Brief summary of commit
##'
##' Displays the first seven characters of the hex, the date and the
##' summary of the commit message:
##' \code{[shortened hex] yyyy-mm-dd: summary}
##' @aliases show,git_commit-methods
##' @docType methods
##' @param object The commit \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @include commit.r
##' @export
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Brief summary of commits in repository
##' invisible(lapply(commits(repo), show))
##' }
##'
setMethod("show",
          signature(object = "git_commit"),
          function (object)
          {
              cat(sprintf("[%s] %s: %s\n",
                          substring(object@hex, 1, 7),
                          substring(as(object@author@when, "character"), 1, 10),
                          object@summary))
          }
)

##' Summary of commit
##'
##' @aliases summary,git_commit-methods
##' @docType methods
##' @param object The commit \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository("path/to/git2r")
##'
##' ## Summary of commit in repository
##' summary(commits(repo)[[1]])
##' }
##'
setMethod("summary",
          signature(object = "git_commit"),
          function(object, ...)
          {
              is_merge_commit <- is_merge(object)
              po <- parents(object)

              cat(sprintf("Commit:  %s\n", object@hex))

              if(is_merge_commit) {
                  hex <- sapply(po, slot, "hex")
                  cat(sprintf("Merge:   %s\n", hex[1]))
                  cat(paste0("         ", hex[-1]), sep="\n")
              }

              cat(sprintf(paste0("Author:  %s <%s>\n",
                                 "When:    %s\n\n"),
                          object@author@name,
                          object@author@email,
                          as(object@author@when, "character")))

              msg <- paste0("    ", readLines(textConnection(object@message)))
              cat(" ", sprintf("%s\n", msg))

              if(is_merge_commit) {
                  lapply(po, function(parent) {
                      msg <- paste0("    ", readLines(textConnection(parent@message)))
                      cat(" ", sprintf("%s\n", msg), "\n")
                  })
              }

              if (identical(length(po), 1L)) {
                  df <- diff(tree(po[[1]]), tree(object))
                  if (length(df) > 0) {
                      plpf <- print_lines_per_file(df)
                      hpf <- hunks_per_file(df)
                      hunk_txt <- ifelse(hpf > 1, " hunks",
                                         ifelse(hpf > 0, " hunk",
                                                " hunk (binary file)"))
                      phpf <- paste0("  in ", format(hpf), hunk_txt)
                      cat(paste0(plpf, phpf), sep="\n")
                  }

                  cat("\n")
              }

              invisible(NULL)
          }
)
