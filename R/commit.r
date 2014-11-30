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
##' config(repo, user.name="Developer", user.email="developer@@example.org")
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
          function (local, upstream)
          {
              stopifnot(identical(local@repo, upstream@repo))
              .Call(git2r_graph_ahead_behind, local, upstream)
          }
)

##' Commit
##'
##' @rdname commit-methods
##' @docType methods
##' @param repo The repository \code{object}.
##' @param message The commit message.
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
##' config(repo, user.name="User", user.email="user@@example.org")
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
                    message = NULL,
                    reference = "HEAD",
                    author = default_signature(repo),
                    committer = default_signature(repo))
           standardGeneric("commit"))

##' @rdname commit-methods
##' @export
setMethod("commit",
          signature(repo = "git_repository"),
          function (repo,
                    message,
                    reference,
                    author,
                    committer)
          {
              ## Argument checking
              stopifnot(is.character(message),
                        identical(length(message), 1L))

              if (!nchar(message[1]))
                  stop("Aborting commit due to empty commit message.")

              .Call(git2r_commit, repo, message, author, committer)
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
##' ## Initialize a repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Config user
##' config(repo, user.name="Author", user.email="author@@example.org")
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
##' @include S4_classes.r
##' @export
setMethod("commits",
          signature(repo = "git_repository"),
          function(repo, topological, time, reverse)
          {
              .Call(git2r_revwalk_list,
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
##' ## Create a directory in tempdir
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##'
##' ## Initialize a repository
##' repo <- init(path)
##' config(repo, user.name="Developer", user.email="developer@@example.org")
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
          function (commit, ancestor)
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
##' config(repo, user.name="User", user.email="user@@example.org")
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
##' @rdname is_merge-methods
##' @docType methods
##' @param commit a S4 class git_commit \code{object}.
##' @return TRUE if commit has more than one parent, else FALSE
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user and commit a file
##' config(repo, user.name="Author", user.email="author@@example.org")
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
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user and commit a file
##' config(repo, user.name="Author", user.email="author@@example.org")
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
              .Call(git2r_commit_parent_list, object)
          }
)

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
##' config(repo, user.name="User", user.email="user@@example.org")
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
          function (object)
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
##' config(repo, user.name="User", user.email="user@@example.org")
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
                  sha <- sapply(po, slot, "sha")
                  cat(sprintf("Merge:   %s\n", sha[1]))
                  cat(paste0("         ", sha[-1]), sep="\n")
              }

              cat(sprintf(paste0("Author:  %s <%s>\n",
                                 "When:    %s\n\n"),
                          object@author@name,
                          object@author@email,
                          as(object@author@when, "character")))

              msg <- paste0("    ", readLines(textConnection(object@message)))
              cat(" ", sprintf("%s\n", msg))

              if (is_merge_commit) {
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
