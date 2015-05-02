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

##' Coerce Git repository to a \code{data.frame}
##'
##' The commits in the repository are coerced to a \code{data.frame}
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
##' @name coerce-git_repository-method
##' @aliases coerce,git_repository,data.frame-method
##' @docType methods
##' @param from The repository \code{object}
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
##' ## Create three files and commit
##' writeLines("First file",  file.path(path, "example-1.txt"))
##' writeLines("Second file", file.path(path, "example-2.txt"))
##' writeLines("Third file",  file.path(path, "example-3.txt"))
##' add(repo, "example-1.txt")
##' commit(repo, "Commit first file")
##' add(repo, "example-2.txt")
##' commit(repo, "Commit second file")
##' add(repo, "example-3.txt")
##' commit(repo, "Commit third file")
##'
##' ## Coerce commits to a data.frame
##' df <- as(repo, "data.frame")
##' df
##' }
setAs(from="git_repository",
      to="data.frame",
      def=function(from)
      {
          do.call("rbind", lapply(commits(from), function(x) {
              data.frame(sha              = x@sha,
                         summary          = x@summary,
                         message          = x@message,
                         author           = x@author@name,
                         email            = x@author@email,
                         when             = as(x@author@when, "POSIXct"),
                         stringsAsFactors = FALSE)
          }))
      }
)

##' Open a repository
##'
##' @rdname repository-methods
##' @docType methods
##' @param path A path to an existing local git repository
##' @param ... Additional arguments to \code{repository} method.
##' @param discover Discover repository from path. Default is FALSE.
##' @return A S4 \code{\linkS4class{git_repository}} object
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' # Configure a user
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Create a file, add and commit
##' writeLines("Hello world!", file.path(path, "test-1.txt"))
##' add(repo, 'test-1.txt')
##' commit_1 <- commit(repo, "Commit message")
##'
##' ## Make one more commit
##' writeLines(c("Hello world!", "HELLO WORLD!"), file.path(path, "test-1.txt"))
##' add(repo, 'test-1.txt')
##' commit(repo, "Next commit message")
##'
##' ## Create one more file
##' writeLines("Hello world!", file.path(path, "test-2.txt"))
##'
##' ## Brief summary of repository
##' repo
##'
##' ## Summary of repository
##' summary(repo)
##'
##' ## Workdir of repository
##' workdir(repo)
##'
##' ## Check if repository is bare
##' is_bare(repo)
##'
##' ## Check if repository is empty
##' is_empty(repo)
##'
##' ## Check if repository is a shallow clone
##' is_shallow(repo)
##'
##' ## List all references in repository
##' references(repo)
##'
##' ## List all branches in repository
##' branches(repo)
##'
##' ## Get HEAD of repository
##' head(repo)
##'
##' ## Check if HEAD is head
##' is_head(head(repo))
##'
##' ## Check if HEAD is local
##' is_local(head(repo))
##'
##' ## List all tags in repository
##' tags(repo)
##' }
setGeneric("repository",
           signature = "path",
           function(path, ...)
           standardGeneric("repository"))

##' @rdname repository-methods
##' @export
setMethod("repository",
          signature(path = "missing"),
          function()
          {
              repository(getwd(), discover = TRUE)
          }
)

##' @rdname repository-methods
##' @export
setMethod("repository",
          signature(path = "character"),
          function(path, discover = FALSE, ...)
          {
              ## Argument checking
              stopifnot(identical(length(path), 1L),
                        nchar(path) > 0,
                        is.logical(discover),
                        identical(length(discover), 1L))

              if (discover) {
                  path <- discover_repository(path)
              } else {
                  path <- normalizePath(path, winslash = "/", mustWork = TRUE)
                  if (!file.info(path)$isdir)
                      stop("'path' is not a directory")
              }

              new("git_repository", path = path)
          }
)

##' Init a repository
##'
##' @rdname init-methods
##' @docType methods
##' @param path A path to where to init a git repository
##' @param bare If TRUE, a Git repository without a working directory
##' is created at the pointed path. If FALSE, provided path will be
##' considered as the working directory into which the .git directory
##' will be created.
##' @return A S4 \code{\linkS4class{git_repository}} object
##' @keywords methods
##' @seealso \link{repository}
##' @examples
##' \dontrun{
##' ## Initialize a repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##' is_bare(repo)
##'
##' ## Initialize a bare repository
##' path_bare <- tempfile(pattern="git2r-")
##' dir.create(path_bare)
##' repo_bare <- init(path_bare, bare = TRUE)
##' is_bare(repo_bare)
##' }
setGeneric("init",
           signature = "path",
           function(path,
                    bare = FALSE)
           standardGeneric("init"))

##' @rdname init-methods
##' @export
setMethod("init",
          signature(path = "character"),
          function(path, bare)
          {
              ## Argument checking
              stopifnot(identical(length(path), 1L),
                        nchar(path) > 0,
                        is.logical(bare),
                        identical(length(bare), 1L))

              path <- normalizePath(path, winslash = "/", mustWork = TRUE)
              if (!file.info(path)$isdir)
                  stop("path is not a directory")

              .Call(git2r_repository_init, path, bare)

              new("git_repository", path=path)
          }
)

##' Clone a remote repository
##'
##' @rdname clone-methods
##' @docType methods
##' @param url The remote repository to clone
##' @param local_path Local directory to clone to.
##' @param bare Create a bare repository. Default is FALSE.
##' @param credentials The credentials for remote repository access. Default
##' is NULL.
##' @param progress Show progress. Default is TRUE.
##' @return A S4 \code{\linkS4class{git_repository}} object
##' @seealso \code{\linkS4class{cred_user_pass}},
##' \code{\linkS4class{cred_ssh_key}}
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Initialize repository
##' path_repo_1 <- tempfile(pattern="git2r-")
##' path_repo_2 <- tempfile(pattern="git2r-")
##' dir.create(path_repo_1)
##' dir.create(path_repo_2)
##' repo_1 <- init(path_repo_1)
##'
##' ## Config user and commit a file
##' config(repo_1, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Write to a file and commit
##' writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'            file.path(path_repo_1, "example.txt"))
##' add(repo_1, "example.txt")
##' commit(repo_1, "First commit message")
##'
##' ## Change file and commit
##' writeLines(c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'              "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
##'            file.path(path_repo_1, "example.txt"))
##' add(repo_1, "example.txt")
##' commit(repo_1, "Second commit message")
##'
##' ## Change file again and commit.
##' writeLines(c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'              "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad",
##'              "minim veniam, quis nostrud exercitation ullamco laboris nisi ut"),
##'            file.path(path_repo_1, "example.txt"))
##' add(repo_1, "example.txt")
##' commit(repo_1, "Third commit message")
##'
##' ## Clone to second repository
##' repo_2 <- clone(path_repo_1, path_repo_2)
##'
##' ## List commits in repositories
##' commits(repo_1)
##' commits(repo_2)
##' }
setGeneric("clone",
           signature = c("url", "local_path"),
           function(url,
                    local_path,
                    bare        = FALSE,
                    credentials = NULL,
                    progress    = TRUE)
           standardGeneric("clone"))

##' @rdname clone-methods
##' @export
setMethod("clone",
          signature(url        = "character",
                    local_path = "character"),
          function(url,
                   local_path,
                   bare,
                   credentials,
                   progress)
          {
              .Call(git2r_clone, url, local_path, bare, credentials, progress)

              repository(local_path)
          }
)

##' Get HEAD for a repository
##'
##' @rdname head-methods
##' @docType methods
##' @param x The repository \code{x} to check head
##' @return NULL if unborn branch or not found. S4 class git_branch if
##' not a detached head. S4 class git_commit if detached head
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Create and initialize a repository in a temporary directory
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Create a file, add and commit
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "Commit message")
##'
##' ## Get HEAD of repository
##' head(repo)
##' }
setMethod("head",
          signature(x = "git_repository"),
          function (x)
          {
              .Call(git2r_repository_head, x)
          }
)

##' Check if repository is bare
##'
##' @rdname is_bare-methods
##' @docType methods
##' @param repo The repository to check if it's bare
##' @return TRUE if bare repository, else FALSE
##' @keywords methods
##' @seealso \link{init}
##' @examples
##' \dontrun{
##' ## Initialize a repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##' is_bare(repo)
##'
##' ## Initialize a bare repository
##' path_bare <- tempfile(pattern="git2r-")
##' dir.create(path_bare)
##' repo_bare <- init(path_bare, bare = TRUE)
##' is_bare(repo_bare)
##' }
setGeneric("is_bare",
           signature = "repo",
           function(repo)
           standardGeneric("is_bare"))

##' @rdname is_bare-methods
##' @export
setMethod("is_bare",
          signature(repo = "git_repository"),
          function (repo)
          {
              .Call(git2r_repository_is_bare, repo)
          }
)

##' @rdname is_bare-methods
##' @export
setMethod("is_bare",
          signature(repo = "missing"),
          function ()
          {
              is_bare(repository(getwd(), discover = TRUE))
          }
)

##' Check if HEAD of repository is detached
##'
##' @rdname is_detached-methods
##' @docType methods
##' @param repo The repository \code{object}
##' @return TRUE if repository HEAD is detached, else FALSE
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Create and initialize a repository in a temporary directory
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Create a file, add and commit
##' writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'            file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit_1 <- commit(repo, "Commit message 1")
##'
##' ## Change file, add and commit
##' writeLines(c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'              "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
##'              file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "Commit message 2")
##'
##' ## HEAD of repository is not detached
##' is_detached(repo)
##'
##' ## Checkout first commit
##' checkout(commit_1)
##'
##' ## HEAD of repository is detached
##' is_detached(repo)
##' }
setGeneric("is_detached",
           signature = "repo",
           function(repo)
           standardGeneric("is_detached"))

##' @rdname is_detached-methods
##' @export
setMethod("is_detached",
          signature(repo = "git_repository"),
          function (repo)
          {
              .Call(git2r_repository_head_detached, repo)
          }
)

##' Check if repository is empty
##'
##' @rdname is_empty-methods
##' @docType methods
##' @param repo The repository to check if it's empty
##' @return TRUE if empty else FALSE
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
##' ## Check if it's an empty repository
##' is_empty(repo)
##'
##' ## Commit a file
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Check if it's an empty repository
##' is_empty(repo)
##' }
setGeneric("is_empty",
           signature = "repo",
           function(repo)
           standardGeneric("is_empty"))

##' @rdname is_empty-methods
##' @export
setMethod("is_empty",
          signature(repo = "git_repository"),
          function (repo)
          {
              .Call(git2r_repository_is_empty, repo)
          }
)

##' Determine if a directory is in a git repository
##'
##' The lookup start from path and walk across parent directories if
##' nothing has been found.
##' @rdname in_repository-methods
##' @docType methods
##' @param path The path to the directory. The working directory is
##' used if path is missing.
##' @return TRUE if directory is in a git repository else FALSE
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
##' ## Check if path is in a git repository
##' in_repository(path)
##'
##' ## Check if working directory is in a git repository
##' setwd(path)
##' in_repository()
##' }
setGeneric("in_repository",
           signature = "path",
           function(path)
           standardGeneric("in_repository"))

##' @rdname in_repository-methods
##' @export
setMethod("in_repository",
          signature(path = "missing"),
          function ()
          {
              !is.null(discover_repository(getwd()))
          }
)

##' @rdname in_repository-methods
##' @export
setMethod("in_repository",
          signature(path = "character"),
          function (path)
          {
              !is.null(discover_repository(path))
          }
)

##' Determine if the repository was a shallow clone
##'
##' @rdname is_shallow-methods
##' @docType methods
##' @param repo The repository
##' @return TRUE if shallow clone, else FALSE
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Initialize repository
##' path_repo_1 <- tempfile(pattern="git2r-")
##' path_repo_2 <- tempfile(pattern="git2r-")
##' dir.create(path_repo_1)
##' dir.create(path_repo_2)
##' repo_1 <- init(path_repo_1)
##'
##' ## Config user and commit a file
##' config(repo_1, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Write to a file and commit
##' writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'            file.path(path_repo_1, "example.txt"))
##' add(repo_1, "example.txt")
##' commit(repo_1, "First commit message")
##'
##' ## Change file and commit
##' writeLines(c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'              "eiusmod tempor incididunt ut labore et dolore magna aliqua."),
##'            file.path(path_repo_1, "example.txt"))
##' add(repo_1, "example.txt")
##' commit(repo_1, "Second commit message")
##'
##' ## Change file again and commit.
##' writeLines(c("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'              "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad",
##'              "minim veniam, quis nostrud exercitation ullamco laboris nisi ut"),
##'            file.path(path_repo_1, "example.txt"))
##' add(repo_1, "example.txt")
##' commit(repo_1, "Third commit message")
##'
##' ## Clone to second repository
##' repo_2 <- clone(path_repo_1, path_repo_2)
##'
##' ## Check if it's a shallow clone
##' is_shallow(repo_2)
##' }
setGeneric("is_shallow",
           signature = "repo",
           function(repo)
           standardGeneric("is_shallow"))

##' @rdname is_shallow-methods
##' @export
setMethod("is_shallow",
          signature(repo = "git_repository"),
          function (repo)
          {
              .Call(git2r_repository_is_shallow, repo)
          }
)

##' Lookup
##'
##' Lookup one object in a repository.
##' @rdname lookup-methods
##' @docType methods
##' @param repo The repository.
##' @param sha The identity of the object to lookup. Must be 4 to 40
##' characters long.
##' @return a \code{git_blob} or \code{git_commit} or \code{git_tag}
##' or \code{git_tree} object
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user and commit a file
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##' writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'            file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit_1 <- commit(repo, "First commit message")
##'
##' ## Create tag
##' tag(repo, "Tagname", "Tag message")
##'
##' ## First, get SHAs to lookup in the repository
##' sha_commit <- commit_1@@sha
##' sha_tree <- tree(commit_1)@@sha
##' sha_blob <- tree(commit_1)["example.txt"]@@sha
##' sha_tag <- tags(repo)[[1]]@@sha
##'
##' ## SHAs
##' sha_commit
##' sha_tree
##' sha_blob
##' sha_tag
##'
##' ## Lookup objects
##' lookup(repo, sha_commit)
##' lookup(repo, sha_tree)
##' lookup(repo, sha_blob)
##' lookup(repo, sha_tag)
##'
##' ## Lookup objects, using only the first seven characters
##' lookup(repo, substr(sha_commit, 1, 7))
##' lookup(repo, substr(sha_tree, 1, 7))
##' lookup(repo, substr(sha_blob, 1, 7))
##' lookup(repo, substr(sha_tag, 1, 7))
##' }
setGeneric("lookup",
           signature = c("repo", "sha"),
           function(repo, sha)
           standardGeneric("lookup"))

##' @rdname lookup-methods
##' @export
setMethod("lookup",
          signature(repo = "git_repository",
                    sha  = "character"),
          function (repo, sha)
          {
              .Call(git2r_object_lookup, repo, sha)
          }
)

##' Get the signature
##'
##' Get the signature according to the repository's configuration
##' @rdname default_signature-methods
##' @docType methods
##' @param repo The repository \code{object} to check signature
##' @return S4 class git_signature
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
##' ## Get the default signature
##' default_signature(repo)
##'
##' ## Change user
##' config(repo, user.name="Bob", user.email="bob@@example.org")
##'
##' ## Get the default signature
##' default_signature(repo)
##' }
setGeneric("default_signature",
           signature = "repo",
           function(repo)
           standardGeneric("default_signature"))

##' @rdname default_signature-methods
##' @export
setMethod("default_signature",
          signature(repo = "git_repository"),
          function (repo)
          {
              .Call(git2r_signature_default, repo)
          }
)

##' Brief summary of repository
##'
##' @aliases show,git_repository-methods
##' @docType methods
##' @param object The repository \code{object}
##' @return None (invisible 'NULL').
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##'
##' ## Brief summary of the repository
##' repo
##'
##' ## Create and commit a file
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Brief summary of the repository
##' repo
##' }
setMethod("show",
          signature(object = "git_repository"),
          function(object)
          {
              lapply(remotes(object), function(remote) {
                  cat(sprintf("Remote:   @ %s (%s)\n",
                              remote,
                              remote_url(object, remote)))
              })

              if (any(is_empty(object), is.null(head(object)))) {
                  cat(sprintf("Local:    %s\n", workdir(object)))
                  cat("Head:     nothing commited (yet)\n")
              } else if (is_detached(object)) {
                  cat(sprintf("Local:    (detached) %s\n", workdir(object)))
              } else {
                  cat(sprintf("Local:    %s %s\n",
                              head(object)@name,
                              workdir(object)))

                  commit <- lookup(object, branch_target(head(object)))
                  cat(sprintf("Head:     [%s] %s: %s\n",
                              substring(commit@sha, 1, 7),
                              substring(as(commit@author@when, "character"), 1, 10),
                              commit@summary))
              }
          }
)

##' Summary of repository
##'
##' @aliases summary,git_repository-methods
##' @docType methods
##' @param object The repository \code{object}
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
##' ## Create a file
##' writeLines("Hello world!", file.path(path, "test.txt"))
##' summary(repo)
##'
##' ## Add file
##' add(repo, "test.txt")
##' summary(repo)
##'
##' ## Commit
##' commit(repo, "First commit message")
##' summary(repo)
##'
##' ## Change the file
##' writeLines(c("Hello again!", "Here is a second line", "And a third"),
##'            file.path(path, "test.txt"))
##' summary(repo)
##'
##' ## Add file and commit
##' add(repo, "test.txt")
##' commit(repo, "Second commit message")
##' summary(repo)
##'}
setMethod("summary",
          signature(object = "git_repository"),
          function(object, ...)
          {
              show(object)
              cat("\n")

              n_branches <- sum(!is.na(unique(sapply(branches(object),
                                                     branch_target))))
              n_tags <- sum(!is.na(unique(sapply(tags(object), slot, "sha"))))

              work <- commits(object)
              n_commits <- length(work)
              n_authors <- length(unique(sapply(lapply(work, slot, "author"),
                                                slot, "name")))

              s <- .Call(git2r_status_list, object, TRUE, TRUE, TRUE, TRUE)
              n_ignored <- length(s$ignored)
              n_untracked <- length(s$untracked)
              n_unstaged <- length(s$unstaged)
              n_staged <- length(s$staged)

              ## Determine max characters needed to display numbers
              n <- max(sapply(c(n_branches, n_tags, n_commits, n_authors,
                                n_ignored, n_untracked, n_unstaged, n_staged),
                              nchar))

              fmt <- paste0("Branches:        %", n, "i\n",
                            "Tags:            %", n, "i\n",
                            "Commits:         %", n, "i\n",
                            "Contributors:    %", n, "i\n",
                            "Ignored files:   %", n, "i\n",
                            "Untracked files: %", n, "i\n",
                            "Unstaged files:  %", n, "i\n",
                            "Staged files:    %", n, "i\n")
              cat(sprintf(fmt, n_branches, n_tags, n_commits, n_authors,
                          n_ignored, n_untracked, n_unstaged, n_staged))

              cat("\nLatest commits:\n")
              lapply(commits(object, n = 5), show)

              invisible(NULL)
          }
)

##' Workdir of repository
##'
##' @rdname workdir-methods
##' @docType methods
##' @param repo The repository \code{object}.
##' @return Character vector with the path of the workdir. If the
##' repository is bare, \code{NULL} will be returned.
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Create a directory in tempdir
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##'
##' ## Initialize a repository
##' repo <- init(path)
##'
##' ## Get the path of the workdir for repository
##' workdir(repo)
##' }
setGeneric("workdir",
           signature = "repo",
           function(repo)
           standardGeneric("workdir"))

##' @rdname workdir-methods
##' @export
setMethod("workdir",
          signature(repo = "git_repository"),
          function (repo)
          {
              .Call(git2r_repository_workdir, repo)
          }
)

##' Find path to repository for any file
##'
##' libgit's git_discover_repository is used to identify the location of the
##'   repository. The path will therefore be terminated by a file separator.
##' @rdname discover_repository-methods
##' @docType methods
##' @param path A character vector specifying the path to a file or folder
##' @return Character vector with path to repository or NULL if this cannot be
##'   established.
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a user and commit a file
##' config(repo, user.name="Alice", user.email="alice@@example.org")
##' writeLines("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do",
##'            file.path(path, "example-1.txt"))
##' add(repo, "example-1.txt")
##' commit(repo, "First commit message")
##'
##' ## Create a second file. The file is not added for version control
##' ## in the repository.
##' dir.create(file.path(path, "example"))
##' file_2 <- file.path(path, "example/example-2.txt")
##' writeLines("Not under version control", file_2)
##'
##' ## Find the path to the repository using the path to the second file
##' discover_repository(file_2)
##' }
setGeneric("discover_repository",
           signature = "path",
           function(path)
           standardGeneric("discover_repository"))

##' @rdname discover_repository-methods
##' @export
setMethod("discover_repository",
          signature(path = "character"),
          function (path)
          {
              .Call(git2r_repository_discover, path)
          }
)
