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

##' Class \code{"git_repository"}
##'
##' @title  S4 class to handle a git repository
##' @section Slots:
##' \describe{
##'   \item{path}{
##'     Path to a git repository
##'   }
##' }
##' @rdname git_repository-class
##' @keywords classes
##' @section Methods:
##' \describe{
##'   \item{is_bare}{\code{signature(object = "git_repository")}}
##'   \item{is_empty}{\code{signature(object = "git_repository")}}
##' }
##' @keywords methods
##' @export
setClass("git_repository",
         slots = c(path = "character"),
         validity = function(object) {
             errors <- character()

             can_open <- .Call(git2r_repository_can_open, object@path)
             if (!isTRUE(can_open))
                 errors <- c(errors, "Unable to open repository at 'path'")

             if (length(errors) == 0) TRUE else errors
         }
)

##' Class \code{"git_time"}
##'
##' @title S4 class to handle a git time in a signature
##' @section Slots:
##' \describe{
##'   \item{time}{
##'     Time in seconds from epoch
##'   }
##'   \item{offset}{
##'     Timezone offset, in minutes
##'   }
##' }
##' @name git_time-class
##' @aliases coerce,git_time,character-method
##' @aliases coerce,git_time,POSIXct-method
##' @aliases show,git_time-method
##' @keywords classes
##' @section Methods:
##' \describe{
##'   \item{show}{\code{signature(object = "git_time")}}
##' }
##' @keywords methods
##' @export
setClass("git_time",
         slots = c(time   = "numeric",
                   offset = "numeric"),
         validity = function(object)
         {
             errors <- character()

             if (!identical(length(object@time), 1L))
                 errors <- c(errors, "time must have length equal to one")
             if (!identical(length(object@offset), 1L))
                 errors <- c(errors, "offset must have length equal to one")

             if (length(errors) == 0) TRUE else errors
         }
)

##' Class \code{"git_signature"}
##'
##' @title S4 class to handle a git signature
##' @section Slots:
##' \describe{
##'   \item{name}{
##'     The full name of the author.
##'   }
##'   \item{email}{
##'     Email of the author.
##'   }
##'   \item{when}{
##'     Time when the action happened.
##'   }
##' }
##' @name git_signature-class
##' @keywords classes
##' @keywords methods
##' @export
setClass("git_signature",
         slots = c(name  = "character",
                   email = "character",
                   when  = "git_time"),
         validity = function(object)
         {
             errors <- validObject(object@when)

             if (isTRUE(errors))
               errors <- character()

             if (!identical(length(object@name), 1L))
                 errors <- c(errors, "name must have length equal to one")
             if (!identical(length(object@email), 1L))
                 errors <- c(errors, "email must have length equal to one")

             if (length(errors) == 0) TRUE else errors
         }
)

##' Class \code{"cred_env"}
##'
##' @title S4 class to handle environmental variables credential
##' object
##' @section Slots:
##' \describe{
##'   \item{username}{
##'     The name of the environmental variable that holds
##'     the username for the authentication.
##'   }
##'   \item{password}{
##'     The name of the environmental variable that holds
##'     the password for the authentication.
##'   }
##' }
##' @rdname cred_env-class
##' @keywords classes
##' @keywords methods
##' @export
setClass("cred_env",
         slots = c(username = "character",
                   password = "character")
)

##' Class \code{"cred_token"}
##'
##' @title S4 class to handle a personal access token credential
##' object
##' @section Slots:
##' \describe{
##'   \item{token}{
##'     The name of the environmental variable that holds
##'     the personal access token for the authentication.
##'   }
##' }
##' @rdname cred_token-class
##' @keywords classes
##' @keywords methods
##' @export
setClass("cred_token",
         slots = c(token = "character")
)

##' Class \code{"cred_user_pass"}
##'
##' @title S4 class to handle plain-text username and password
##' credential object
##' @section Slots:
##' \describe{
##'   \item{username}{
##'     The username of the credential
##'   }
##'   \item{password}{
##'     The password of the credential
##'   }
##' }
##' @rdname cred_user_pass-class
##' @keywords classes
##' @keywords methods
##' @export
setClass("cred_user_pass",
         slots = c(username = "character",
                   password = "character")
)

##' Class \code{"cred_ssh_key"}
##'
##' @title S4 class to handle a passphrase-protected ssh key
##' credential object
##' @section Slots:
##' \describe{
##'   \item{publickey}{
##'     The path to the public key of the credential
##'   }
##'   \item{privatekey}{
##'     The path to the private key of the credential
##'   }
##'   \item{passphrase}{
##'     The passphrase of the credential
##'   }
##' }
##' @rdname cred_ssh_key-class
##' @keywords classes
##' @keywords methods
##' @export
setClass("cred_ssh_key",
         slots = c(publickey  = "character",
                   privatekey = "character",
                   passphrase = "character")
)

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
##' @keywords classes
##' @keywords methods
##' @export
setClass("git_blame",
         slots = c(path  = "character",
                   hunks = "list",
                   repo  = "git_repository"),
         validity = function(object) {
             errors <- character()

             if (length(errors) == 0) TRUE else errors
         }
)

##' Class \code{"git_blame_hunk"}
##'
##' @title  S4 class to represent a blame hunk
##' @section Slots:
##' \describe{
##'   \item{lines_in_hunk}{
##'     The number of lines in this hunk
##'   }
##'   \item{final_commit_id}{
##'     The sha of the commit where this line was last changed
##'   }
##'   \item{final_start_line_number}{
##'     The 1-based line number where this hunk begins, in the final
##'     version of the file
##'   }
##'   \item{final_signature}{
##'     Final committer
##'   }
##'   \item{orig_commit_id}{
##'     The sha of the commit where this hunk was found. This will usually
##'     be the same as 'final_commit_id'.
##'   }
##'   \item{orig_start_line_number}{
##'      The 1-based line number where this hunk begins in the file
##'      named by 'orig_path' in the commit specified by 'orig_commit_id'.
##'   }
##'   \item{orig_signature}{
##'     Origin committer
##'   }
##'   \item{orig_path}{
##'     The path to the file where this hunk originated, as of the commit
##'     specified by 'orig_commit_id'
##'   }
##'   \item{boundary}{
##'     TRUE iff the hunk has been tracked to a boundary commit.
##'   }
##'   \item{repo}{
##'     The S4 class git_repository that contains the blame hunk
##'   }
##' }
##' @rdname git_blame_hunk-class
##' @keywords classes
##' @keywords methods
##' @export
setClass("git_blame_hunk",
         slots = c(lines_in_hunk           = "integer",
                   final_commit_id         = "character",
                   final_start_line_number = "integer",
                   final_signature         = "git_signature",
                   orig_commit_id          = "character",
                   orig_start_line_number  = "integer",
                   orig_signature          = "git_signature",
                   orig_path               = "character",
                   boundary                = "logical",
                   repo                    = "git_repository"),
         validity = function(object) {
             errors <- character()

             if (length(errors) == 0) TRUE else errors
         }
)

##' Class \code{"git_blob"}
##'
##' @title  S4 class to handle a git blob
##' @section Slots:
##' \describe{
##'   \item{sha}{
##'     The 40 character hexadecimal string of the SHA-1
##'   }
##'   \item{repo}{
##'     The S4 class git_repository that contains the blob
##'   }
##' }
##' @rdname git_blob-class
##' @keywords classes
##' @keywords methods
##' @export
setClass("git_blob",
         slots = c(sha  = "character",
                   repo = "git_repository"),
         validity = function(object) {
             errors <- character()

             if (length(errors) == 0) TRUE else errors
         }
)

##' Class \code{git_branch}
##'
##' @title S4 class to handle a git branch
##' @section Slots:
##' \describe{
##'   \item{name}{
##'     Name of the branch.
##'   }
##'   \item{type}{
##'     Type of the branch, either 1 (local) or 2 (remote).
##'   }
##'   \item{repo}{
##'     The S4 class git_repository that contains the branch
##'   }
##' }
##' @name git_branch-class
##' @keywords classes
##' @section Methods:
##' \describe{
##'   \item{is_head}{\code{signature(object = "git_branch")}}
##'   \item{is_local}{\code{signature(object = "git_branch")}}
##'   \item{show}{\code{signature(object = "git_branch")}}
##' }
##' @keywords methods
##' @export
setClass("git_branch",
         slots = c(name = "character",
                   type = "integer",
                   repo = "git_repository"))

##' Class \code{"git_commit"}
##'
##' @title S4 class to handle a git commit.
##' @section Slots:
##' \describe{
##'   \item{sha}{
##'     The 40 character hexadecimal string of the SHA-1
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
##' @keywords classes
##' @section Methods:
##' \describe{
##'   \item{show}{\code{signature(object = "git_commit")}}
##' }
##' @keywords methods
##' @export
setClass("git_commit",
         slots = c(sha       = "character",
                   author    = "git_signature",
                   committer = "git_signature",
                   summary   = "character",
                   message   = "character",
                   repo      = "git_repository"),
         prototype = list(summary = NA_character_,
                          message = NA_character_))

##' Git diff
##'
##' @section Slots:
##' \describe{
##'   \item{old}{
##'     Represents the 'from' side of the diff.
##'   }
##'   \item{new}{
##'     Represents the 'to' side of the diff.
##'   }
##'   \item{files}{
##'     List of \code{\linkS4class{git_diff_file}} objects.
##'   }
##' }
##' @name git_diff-class
##' @keywords classes
##' @export
setClass("git_diff",
         slots = c(old   = "ANY",
                   new   = "ANY",
                   files = "list"),
         prototype = list(old = NA_character_,
                          new = NA_character_))

##' Git diff file
##'
##' @section Slots:
##' \describe{
##'   \item{old_file}{
##'     Filename for old side of diff.
##'   }
##'   \item{new_file}{
##'     Filename for new side of diff.
##'   }
##'   \item{hunks}{
##'     List of \code{\linkS4class{git_diff_hunk}} objects.
##'   }
##' }
##' @name git_diff_file-class
##' @keywords classes
##' @export
setClass("git_diff_file",
         slots = c(old_file = "character",
                   new_file = "character",
                   hunks    = "list"))

##' Git diff hunk
##'
##' @section Slots:
##' \describe{
##'   \item{old_start}{
##'     Starting line number in old_file.
##'   }
##'   \item{old_lines}{
##'     Number of lines in old_file.
##'   }
##'   \item{new_start}{
##'     Starting line number in new_file.
##'   }
##'   \item{new_lines}{
##'     Number of lines in new_file.
##'   }
##'   \item{header}{
##'     Header text.
##'   }
##'   \item{lines}{
##'     List with \code{\linkS4class{git_diff_line}} objects.
##'   }
##' }
##' @name git_diff_hunk-class
##' @keywords classes
##' @export
setClass("git_diff_hunk",
         slots = c(old_start = "integer",
                   old_lines = "integer",
                   new_start = "integer",
                   new_lines = "integer",
                   header    = "character",
                   lines     = "list"))

##' Git diff line
##'
##' @section Slots:
##' \describe{
##'   \item{origin}{
##'     Line origin constants.
##'   }
##'   \item{old_lineno}{
##'     Line number in old file or -1 for added line.
##'   }
##'   \item{new_lineno}{
##'     Line number in new file or -1 for deleted line.
##'   }
##'   \item{num_lines}{
##'     Number of newline characters in content.
##'   }
##'   \item{content}{
##'     The diff text.
##'   }
##' }
##' @name git_diff_line-class
##' @keywords classes
##' @export
setClass("git_diff_line",
         slots = c(origin     = "integer",
                   old_lineno = "integer",
                   new_lineno = "integer",
                   num_lines  = "integer",
                   content    = "character"))

##' Class \code{git_note}
##'
##' @title S4 class to handle a git note
##' @section Slots:
##' \describe{
##'   \item{sha}{
##'     The 40 character hexadecimal string of the SHA-1 of the blob
##'     containing the message
##'   }
##'   \item{annotated}{
##'     The 40 character hexadecimal string of the SHA-1 of the git
##'     object being annotated
##'   }
##'   \item{message}{
##'     The note message
##'   }
##'   \item{refname}{
##'     Name of the reference
##'   }
##'   \item{repo}{
##'     The S4 class git_repository that contains the note
##'   }
##' }
##' @name git_note-class
##' @keywords classes
##' @keywords methods
##' @export
setClass("git_note",
         slots = c(sha       = "character",
                   annotated = "character",
                   message   = "character",
                   refname   = "character",
                   repo      = "git_repository"))

##' Class \code{"git_reference"}
##'
##' @title S4 class to handle a git reference
##' @section Slots:
##' \describe{
##'   \item{name}{
##'     The full name of the reference.
##'   }
##'   \item{type}{
##'     Type of the reference, either direct (GIT_REF_OID == 1) or
##'     symbolic (GIT_REF_SYMBOLIC == 2)
##'   }
##'   \item{sha}{
##'     The 40 character hexadecimal string of the SHA-1
##'   }
##'   \item{target}{
##'     The full name to the reference pointed to by a
##'     symbolic reference.
##'   }
##'   \item{shorthand}{
##'     The reference's short name
##'   }
##' }
##' @rdname git_reference-class
##' @keywords classes
##' @section Methods:
##' \describe{
##'   \item{show}{\code{signature(object = "git_reference")}}
##' }
##' @keywords methods
##' @export
setClass("git_reference",
         slots = c(name      = "character",
                   type      = "integer",
                   sha       = "character",
                   target    = "character",
                   shorthand = "character"),
         prototype = list(sha    = NA_character_,
                          target = NA_character_))

##' Class \code{"git_reflog_entry"}
##'
##' @title S4 class to handle a git reflog entry.
##' @section Slots:
##' \describe{
##'   \item{sha}{
##'     The 40 character hexadecimal string of the SHA-1
##'   }
##'   \item{message}{
##'     The log message of the entry
##'   }
##'   \item{index}{
##'     The index (zero-based) of the entry in the reflog
##'   }
##'   \item{committer}{
##'     The committer signature
##'   }
##'   \item{refname}{
##'     Name of the reference
##'   }
##'   \item{repo}{
##'     The S4 class git_repository that contains the reflog entry
##'   }
##' }
##' @name git_reflog_entry-class
##' @keywords classes
##' @keywords methods
##' @export
setClass("git_reflog_entry",
         slots = c(sha       = "character",
                   message   = "character",
                   index     = "integer",
                   committer = "git_signature",
                   refname   = "character",
                   repo      = "git_repository"))

##' Class \code{"git_stash"}
##'
##' @title S4 class to handle a git stash
##' @name git_stash-class
##' @keywords classes
##' @keywords methods
##' @export
setClass("git_stash", contains = "git_commit")

##' Class \code{"git_tag"}
##'
##' @title S4 class to handle a git tag
##' @section Slots:
##' \describe{
##'   \item{sha}{
##'     The 40 character hexadecimal string of the SHA-1
##'   }
##'   \item{message}{
##'     The message of the tag
##'   }
##'   \item{name}{
##'     The name of the tag
##'   }
##'   \item{tagger}{
##'     The tagger (author) of the tag
##'   }
##'   \item{target}{
##'     The target of the tag
##'   }
##'   \item{repo}{
##'     The S4 class git_repository that contains the tag
##'   }
##' }
##' @name git_tag-class
##' @keywords classes
##' @keywords methods
##' @export
setClass("git_tag",
         slots = c(sha     = "character",
                   message = "character",
                   name    = "character",
                   tagger  = "git_signature",
                   target  = "character",
                   repo    = "git_repository"),
         validity = function(object)
         {
             errors <- validObject(object@tagger)

             if (isTRUE(errors))
               errors <- character()

             if (!identical(length(object@sha), 1L))
                 errors <- c(errors, "sha must have length equal to one")
             if (!identical(length(object@message), 1L))
                 errors <- c(errors, "message must have length equal to one")
             if (!identical(length(object@name), 1L))
                 errors <- c(errors, "name must have length equal to one")
             if (!identical(length(object@target), 1L))
                 errors <- c(errors, "target must have length equal to one")

             if (length(errors) == 0) TRUE else errors
         }
)

##' Class \code{"git_tree"}
##'
##' @title S4 class to handle a git tree
##' @section Slots:
##' \describe{
##'   \item{sha}{
##'     The 40 character hexadecimal string of the SHA-1
##'   }
##'   \item{filemode}{
##'     The UNIX file attributes of a tree entry
##'   }
##'   \item{type}{
##'     String representation of the tree entry type
##'   }
##'   \item{id}{
##'     The sha id of a tree entry
##'   }
##'   \item{name}{
##'     The filename of a tree entry
##'   }
##'   \item{repo}{
##'     The S4 class git_repository that contains the commit
##'   }
##' }
##' @name git_tree-class
##' @keywords classes
##' @keywords methods
##' @export
setClass("git_tree",
         slots = c(sha      = "character",
                   filemode = "integer",
                   type     = "character",
                   id       = "character",
                   name     = "character",
                   repo     = "git_repository"),
         validity = function(object)
         {
             errors <- character(0)

             if (!identical(length(object@sha), 1L))
                 errors <- c(errors, "sha must have length equal to one")

             if (length(errors) == 0) TRUE else errors
         }
)

##' Class \code{"git_transfer_progress"}
##'
##' Statistics from the fetch operation.
##' @section Slots:
##' \describe{
##'   \item{total_objects}{
##'     Number of objects in the packfile being downloaded
##'   }
##'   \item{indexed_objects}{
##'     Received objects that have been hashed
##'   }
##'   \item{received_objects}{
##'     Objects which have been downloaded
##'   }
##'   \item{total_deltas}{
##'     Total number of deltas in the pack
##'   }
##'   \item{indexed_deltas}{
##'     Deltas which have been indexed
##'   }
##'   \item{local_objects}{
##'     Locally-available objects that have been injected in order to
##'     fix a thin pack
##'   }
##'   \item{received_bytes}{
##'     Size of the packfile received up to now
##'   }
##' }
##' @name git_transfer_progress-class
##' @keywords classes
##' @keywords methods
##' @export
setClass("git_transfer_progress",
         slots = c(total_objects    = "integer",
                   indexed_objects  = "integer",
                   received_objects = "integer",
                   local_objects    = "integer",
                   total_deltas     = "integer",
                   indexed_deltas   = "integer",
                   received_bytes   = "integer"))

##' Class \code{"git_fetch_head"}
##'
##' @title S4 class to handle a fetch head
##' @section Slots:
##' \describe{
##'   \item{ref_name}{
##'     The name of the ref.
##'   }
##'   \item{remote_url}{
##'     The url of the remote.
##'   }
##'   \item{sha}{
##'     The SHA-1 of the remote head that were updated during the last
##'     fetch.
##'   }
##'   \item{is_merge}{
##'     Is head for merge.
##'   }
##'   \item{repo}{
##'     The S4 class git_repository that contains the fetch head.
##'   }
##' }
##' @rdname git_fetch_head
##' @keywords classes
##' @keywords methods
##' @export
setClass("git_fetch_head",
         slots = c(ref_name   = "character",
                   remote_url = "character",
                   sha        = "character",
                   is_merge   = "logical",
                   repo       = "git_repository")
)

##' Class \code{"git_merge_result"}
##'
##' @title S4 class to handle the merge result
##' @section Slots:
##' \describe{
##'   \item{up_to_date}{
##'     TRUE if the merge is already up-to-date, else FALSE.
##'   }
##'   \item{fast_forward}{
##'     TRUE if a fast-forward merge, else FALSE.
##'   }
##'   \item{conflicts}{
##'     TRUE if the index contain entries representing file conflicts,
##'     else FALSE.
##'   }
##'   \item{sha}{
##'     If the merge created a merge commit, the sha of the merge
##'     commit. NA if no merge commit created.
##'   }
##' }
##' @rdname git_merge_result
##' @keywords classes
##' @keywords methods
##' @export
setClass("git_merge_result",
         slots = c(up_to_date   = "logical",
                   fast_forward = "logical",
                   conflicts    = "logical",
                   sha          = "character")
)

#' @importFrom methods setClassUnion
setClassUnion("git_credentials", c("NULL", "cred_user_pass", "cred_ssh_key"))

#' The git_connection class
#'
#' @section Slots:
#'   \describe{
#'    \item{\code{Repository}}{a git repository}
#'    \item{\code{LocalPath}}{a subdirectory wihtin the repository}
#'    \item{\code{Credentials}}{the credentials for the repository}
#'    \item{\code{CommitUser}}{the name of the user how will commit}
#'    \item{\code{CommitEmail}}{the email of the user how will commit}
#'   }
#' @name git_connection-class
#' @rdname git_connection-class
#' @exportClass git_connection
#' @aliases git_connection-class
#' @importFrom methods setClass
#' @docType class
#' @template thierry
setClass(
  Class = "git_connection",
  representation = representation(
    Repository = "git_repository",
    LocalPath = "character",
    Credentials = "git_credentials",
    CommitUser = "character",
    CommitEmail = "character"
  )
)

#' @importFrom methods setValidity
#' @importFrom assertthat assert_that is.string has_name
#' @importFrom utils file_test
setValidity(
  "git_connection",
  function(object){
    assert_that(is.string(object@CommitUser))
    assert_that(is.string(object@CommitEmail))
    root <- normalizePath(object@Repository@path)
    normalizePath(paste(root, object@LocalPath, sep = "/"))
    repo <- repository(root)
    repo.config <- config(repo)
    assert_that(has_name(repo.config$local, "user.name"))
    assert_that(has_name(repo.config$local, "user.email"))
    assert_that(repo.config$local$user.name == object@CommitUser)
    assert_that(repo.config$local$user.email == object@CommitEmail)
    return(TRUE)
  }
)
