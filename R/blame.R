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

##' Get blame for file
##'
##' @template repo-param
##' @param path Path to the file to consider
##' @return git_blame object with the following entries:
##' \describe{
##'   \item{path}{
##'     The path to the file of the blame
##'   }
##'   \item{hunks}{
##'     List of blame hunks
##'   }
##'   \item{repo}{
##'     The git_repository that contains the file
##'   }
##' }
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
##'     The \code{git_repository} object that contains the blame hunk
##'   }
##' }
##' @export
##' @examples
##' \dontrun{
##' ## Initialize a temporary repository
##' path <- tempfile(pattern="git2r-")
##' dir.create(path)
##' repo <- init(path)
##'
##' ## Create a first user and commit a file
##' config(repo, user.name = "Alice", user.email = "alice@@example.org")
##' writeLines("Hello world!", file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "First commit message")
##'
##' ## Create a second user and change the file
##' config(repo, user.name = "Bob", user.email = "bob@@example.org")
##' writeLines(c("Hello world!", "HELLO WORLD!", "HOLA"),
##'            file.path(path, "example.txt"))
##' add(repo, "example.txt")
##' commit(repo, "Second commit message")
##'
##' ## Check blame
##' blame(repo, "example.txt")
##' }
blame <- function(repo = ".", path = NULL) {
    .Call(git2r_blame_file, lookup_repository(repo), path)
}
