## git2r, R bindings to the libgit2 library.
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
##' @examples \dontrun{
##' ## Create a directory in tempdir
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##'
##' ## Initialize a repository
##' repo <- init(path)
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Create a file, add and commit
##' writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'            con = file.path(path, "test.txt"))
##' add(repo, "test.txt")
##' commit_1 <- commit(repo, "Commit message 1")
##'
##' # Change file and commit
##' writeLines(c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'              "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
##'              con = file.path(path, "test.txt"))
##' add(repo, "test.txt")
##' commit_2 <- commit(repo, "Commit message 2")
##'
##' ahead_behind(commit_1, commit_2)
##' }
setGeneric("ahead_behind",
           signature = c("local", "upstream"),
           function(local, upstream)
           standardGeneric("ahead_behind"))

##' @rdname ahead_behind-methods
##' @export
setMethod("ahead_behind",
          signature(local = "git_commit", upstream = "git_commit"),
          function(local, upstream)
          {
              stopifnot(identical(local@repo, upstream@repo))
              .Call(git2r_graph_ahead_behind, local, upstream)
          }
)

##' Add sessionInfo to message
##'
##' @param message The message.
##' @return message with appended sessionInfo
##' @importFrom utils capture.output
##' @importFrom utils sessionInfo
##' @noRd
add_session_info <- function(message)
{
    paste0(message, "\n\nsessionInfo:\n",
           paste0(utils::capture.output(utils::sessionInfo()),
                  collapse="\n"))
}

##' Commit
##'
##' @rdname commit-methods
##' @docType methods
##' @param repo The repository \code{object}.
##' @param message The commit message.
##' @param all Stage modified and deleted files. Files not added to
##' Git are not affected.
##' @param session Add sessionInfo to commit message. Default is
##' FALSE.
##' @param reference Name of the reference that will be updated to
##' point to this commit.
##' @param author Signature with author and author time of commit.
##' @param committer Signature with committer and commit time of commit.
##' @return \code{\linkS4class{git_commit}} object
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Initialize a repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Config user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Write to a file and commit
##' writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'            file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##' }
setGeneric("commit",
           signature = "repo",
           function(repo,
                    message   = NULL,
                    all       = FALSE,
                    session   = FALSE,
                    reference = "HEAD",
                    author    = default_signature(repo),
                    committer = default_signature(repo))
           standardGeneric("commit"))

##' @rdname commit-methods
##' @export
setMethod("commit",
          signature(repo = "git_repository"),
          function(repo,
                   message,
                   all,
                   session,
                   reference,
                   author,
                   committer)
          {
              ## Argument checking
              stopifnot(is.character(message),
                        identical(length(message), 1L),
                        is.logical(all),
                        identical(length(all), 1L),
                        is.logical(session),
                        identical(length(session), 1L))

              if (!nchar(message[1]))
                  stop("Aborting commit due to empty commit message.")

              if (all) {
                  s <- status(repo,
                              unstaged  = TRUE,
                              staged    = FALSE,
                              untracked = FALSE,
                              ignored   = FALSE)

                  # Convert list of lists to character vector
                  unstaged <- unlist(s$unstaged)
                  for (i in seq_along(unstaged)) {
                    if (names(unstaged)[i] == "modified") {
                      ## Stage modified files
                      add(repo, unstaged[i])
                    } else if (names(unstaged)[i] == "deleted") {
                      ## Stage deleted files
                      .Call(git2r_index_remove_bypath, repo, unstaged[i])
                    }
                  }

              }

              if (session)
                  message <- add_session_info(message)

              .Call(git2r_commit, repo, message, author, committer)
          }
)

##' Commits
##'
##' @rdname commits-methods
##' @docType methods
##' @param repo The repository \code{object}
##' \code{\linkS4class{git_repository}}. If the \code{repo} argument
##' is missing, the repository is searched for with
##' \code{\link{discover_repository}} in the current working
##' directory.
##' @param ... Additional arguments to commits.
##' @param topological Sort the commits in topological order (parents
##' before children); can be combined with time sorting. Default is
##' TRUE.
##' @param time Sort the commits by commit time; Can be combined with
##' topological sorting. Default is TRUE.
##' @param reverse Sort the commits in reverse order; can be combined
##' with topological and/or time sorting. Default is FALSE.
##' @param n The upper limit of the number of commits to output. The
##' defualt is NULL for unlimited number of commits.
##' @return list of commits in repository
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Initialize a repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Config user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Write to a file and commit
##' writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'            file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Change file and commit
##' writeLines(c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'              "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
##'            file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "Second commit message")
##'
##' ## Change file again and commit
##' writeLines(c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'              "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad",
##'              "minim veniam, quis nostrud exercitation ullamco laboris nisi ut"),
##'            file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "Third commit message")
##'
##' ## List commits in repository
##' commits(repo)
##' }
setGeneric("commits",
           signature = "repo",
           function(repo, ...)
           standardGeneric("commits"))

##' @rdname commits-methods
##' @export
setMethod("commits",
          signature(repo = "missing"),
          function(topological = TRUE,
                   time        = TRUE,
                   reverse     = FALSE,
                   n           = NULL,
                   ...)
          {
              callGeneric(repo        = lookup_repository(),
                          topological = topological,
                          time        = time,
                          reverse     = reverse,
                          n           = n,
                          ...)
          }
)

##' @rdname commits-methods
##' @include S4_classes.r
##' @export
setMethod("commits",
          signature(repo = "git_repository"),
          function(repo,
                   topological = TRUE,
                   time        = TRUE,
                   reverse     = FALSE,
                   n           = NULL,
                   ...)
          {
              ## Check limit in number of commits
              if (is.null(n)) {
                  n <- -1L
              } else if (is.numeric(n)) {
                  if (!identical(length(n), 1L))
                      stop("'n' must be integer")
                  if (abs(n - round(n)) >= .Machine$double.eps^0.5)
                      stop("'n' must be integer")
                  n <- as.integer(n)
              } else {
                  stop("'n' must be integer")
              }

              if (is_shallow(repo)) {
                  ## FIXME: Remove this if-statement when libgit2
                  ## supports shallow clones, see #219.  Note: This
                  ## workaround does not use the 'topological', 'time'
                  ## and 'reverse' flags.

                  ## List to hold result
                  result <- list()

                  ## Get latest commit
                  x <- lookup(repo, branch_target(head(repo)))

                  ## Repeat until no more parent commits
                  repeat {
                      if (n == 0) {
                          break
                      } else if (n > 0) {
                          n <- n - 1
                      }

                      if (is.null(x))
                          break
                      result <- append(result, x)

                      ## Get parent to commit
                      x <- tryCatch(parents(x)[[1]], error = function(e) NULL)
                  }

                  return(result)
              }

              .Call(git2r_revwalk_list, repo, topological, time, reverse, n)
          }
)

##' Last commit
##'
##' Get last commit in the current branch.
##' @template repo-param
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Config user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Write to a file and commit
##' writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'            file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Get last commit
##' last_commit(repo)
##' last_commit(path)
##' }
last_commit <- function(repo = NULL) {
    if (is.null(repo)) {
        repo <- lookup_repository()
    } else if (is.character(repo)) {
        repo <- repository(repo)
    }
    commits(repo, n = 1)[[1]]
}

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
##' ## Create a directory in tempdir
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##'
##' ## Initialize a repository
##' repo <- init(path)
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Create a file, add and commit
##' writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'            con = file.path(path, "test.txt"))
##' add(repo, "test.txt")
##' commit_1 <- commit(repo, "Commit message 1")
##'
##' # Change file and commit
##' writeLines(c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'              "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
##'              con = file.path(path, "test.txt"))
##' add(repo, "test.txt")
##' commit_2 <- commit(repo, "Commit message 2")
##'
##' descendant_of(commit_1, commit_2)
##' descendant_of(commit_2, commit_1)
##' }
setGeneric("descendant_of",
           signature = c("commit", "ancestor"),
           function(commit, ancestor)
           standardGeneric("descendant_of"))

##' @rdname descendant_of-methods
##' @export
setMethod("descendant_of",
          signature(commit = "git_commit", ancestor = "git_commit"),
          function(commit, ancestor)
          {
              stopifnot(identical(commit@repo, ancestor@repo))
              .Call(git2r_graph_descendant_of, commit, ancestor)
          }
)

##' Check if object is S4 class git_commit
##'
##' @param object Check if object is S4 class git_commit
##' @return TRUE if object is S4 class git_commit, else FALSE
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Commit a text file
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit_1 <- commit(repo, "First commit message")
##'
##' ## Check if commit
##' is_commit(commit_1)
##' }
is_commit <- function(object) {
    is(object = object, class2 = "git_commit")
}

##' Is merge
##'
##' Determine if a commit is a merge commit, i.e. has more than one
##' parent.
##' @param commit a S4 class git_commit \code{object}.
##' @return TRUE if commit has more than one parent, else FALSE
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user and commit a file
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##' writeLines(c("First line in file 1.", "Second line in file 1."),
##'            file.path(path, "example-1.txt"))
##' add(repo, "example-1.txt")
##' commit(repo, "First commit message")
##'
##' ## Create and add one more file
##' writeLines(c("First line in file 2.", "Second line in file 2."),
##'            file.path(path, "example-2.txt"))
##' add(repo, "example-2.txt")
##' commit(repo, "Second commit message")
##'
##' ## Create a new branch 'fix'
##' checkout(repo, "fix", create = TRUE)
##'
##' ## Update 'example-1.txt' (swap words in first line) and commit
##' writeLines(c("line First in file 1.", "Second line in file 1."),
##'            file.path(path, "example-1.txt"))
##' add(repo, "example-1.txt")
##' commit(repo, "Third commit message")
##'
##' checkout(repo, "master")
##'
##' ## Update 'example-2.txt' (swap words in second line) and commit
##' writeLines(c("First line in file 2.", "line Second in file 2."),
##'            file.path(path, "example-2.txt"))
##' add(repo, "example-2.txt")
##' commit(repo, "Fourth commit message")
##'
##' ## Merge 'fix'
##' merge(repo, "fix")
##'
##' ## Display parents of last commit
##' parents(lookup(repo, branch_target(head(repo))))
##'
##' ## Check that last commit is a merge
##' is_merge(lookup(repo, branch_target(head(repo))))
##' }
is_merge <- function(commit = NULL)
{
    length(parents(commit)) > 1
}

##' Parents
##'
##' Get parents of a commit.
##' @param object a S4 class git_commit \code{object}.
##' @return list of S4 git_commit objects
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user and commit a file
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##' writeLines("First line.",
##'            file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit_1 <- commit(repo, "First commit message")
##'
##' ## commit_1 has no parents
##' parents(commit_1)
##'
##' ## Update 'example.txt' and commit
##' writeLines(c("First line.", "Second line."),
##'            file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit_2 <- commit(repo, "Second commit message")
##'
##' ## commit_2 has commit_1 as parent
##' parents(commit_2)
##' }
parents <- function(object = NULL)
{
    .Call(git2r_commit_parent_list, object)
}

##' Brief summary of commit
##'
##' Displays the first seven characters of the sha, the date and the
##' summary of the commit message:
##' \code{[shortened sha] yyyy-mm-dd: summary}
##' @aliases show,git_commit-methods
##' @docType methods
##' @param object The commit \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @include S4_classes.r
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Config user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Write to a file and commit
##' writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'            file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Brief summary of commit in repository
##' show(commits(repo)[[1]])
##' }
setMethod("show",
          signature(object = "git_commit"),
          function(object)
          {
              cat(sprintf("[%s] %s: %s\n",
                          substring(object@sha, 1, 7),
                          substring(as(object@author@when, "character"), 1, 10),
                          object@summary))
          }
)

##' Summary of commit
##'
##' @aliases summary,git_commit-methods
##' @docType methods
##' @param object The commit \code{object}
##' @param ... Additional arguments affecting the summary produced.
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Config user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Write to a file and commit
##' writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'            file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Summary of commit in repository
##' summary(commits(repo)[[1]])
##' }
setMethod("summary",
          signature(object = "git_commit"),
          function(object, ...)
          {
              is_merge_commit <- is_merge(object)
              po <- parents(object)

              cat(sprintf("Commit:  %s\n", object@sha))

              if (is_merge_commit) {
                  sha <- vapply(po, slot, character(1), "sha")
                  cat(sprintf("Merge:   %s\n", sha[1]))
                  cat(paste0("         ", sha[-1]), sep="\n")
              }

              cat(sprintf(paste0("Author:  %s <%s>\n",
                                 "When:    %s\n\n"),
                          object@author@name,
                          object@author@email,
                          as(object@author@when, "character")))

              msg <- paste0("    ", readLines(textConnection(object@message)))
              cat("", sprintf("%s\n", msg))

              if (is_merge_commit) {
                  cat("\n")
                  lapply(po, function(parent) {
                      cat("Commit message: ", parent@sha, "\n")
                      msg <- paste0("    ",
                                    readLines(textConnection(parent@message)))
                      cat("", sprintf("%s\n", msg), "\n")
                  })
              }

              if (identical(length(po), 1L)) {
                  df <- diff(tree(po[[1]]), tree(object))
                  if (length(df) > 0) {
                      if (length(df) > 1) {
                          cat(sprintf("%i files changed, ", length(df)))
                      } else {
                          cat("1 file changed, ")
                      }

                      cat(sprintf("%i insertions, %i deletions\n",
                                  sum(vapply(lines_per_file(df), "[[", numeric(1), "add")),
                                  sum(vapply(lines_per_file(df), "[[", numeric(1), "del"))))

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

##' Coerce a commit to a \code{data.frame}
##'
##' The commit is coerced to a \code{data.frame}
##'
##'
##' The \code{data.frame} have the following columns:
##' \describe{
##'
##'   \item{sha}{
##'     The 40 character hexadecimal string of the SHA-1
##'   }
##'
##'   \item{summary}{
##'     the short "summary" of the git commit message.
##'   }
##'
##'   \item{message}{
##'     the full message of a commit
##'   }
##'
##'   \item{author}{
##'     full name of the author
##'   }
##'
##'   \item{email}{
##'     email of the author
##'   }
##'
##'   \item{when}{
##'     time when the commit happened
##'   }
##'
##' }
##' @name coerce-git_commit-method
##' @aliases coerce,git_commit,data.frame-method
##' @docType methods
##' @param from The commit \code{object}
##' @return \code{data.frame}
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Create a file and commit
##' writeLines("Example file",  file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' c1 <- commit(repo, "Commit message")
##'
##' ## Coerce the commit to a data.frame
##' df <- as(c1, "data.frame")
##' df
##' }
setAs(from = "git_commit",
      to   = "data.frame",
      def  = function(from)
      {
          data.frame(sha              = from@sha,
                     summary          = from@summary,
                     message          = from@message,
                     author           = from@author@name,
                     email            = from@author@email,
                     when             = as(from@author@when, "POSIXct"),
                     stringsAsFactors = FALSE)
      }
)
