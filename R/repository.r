## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013  Stefan Widgren
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, version 2 of the License.
##
## git2r is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.

##' Class \code{"repository"}
##'
##' S4 class to handle a git repository
##' @section Slots:
##' \describe{
##'   \item{repo}{
##'     External pointer to a git repository
##'   }
##' }
##' @name repository-class
##' @aliases show,repository-method
##' @aliases summary,repository-method
##' @docType class
##' @keywords classes
##' @section Methods:
##' \describe{
##'   \item{is.bare}{\code{signature(object = "repository")}}
##'   \item{is.empty}{\code{signature(object = "repository")}}
##'   \item{show}{\code{signature(object = "repository")}}
##'   \item{summary}{\code{signature(object = "repository")}}
##' }
##' @keywords methods
##' @export
setClass('repository',
         slots=c(repo='externalptr'))

##' Open a repository
##'
##' @param path A path to an existing local git repository
##' @return A S4 \code{repository} object
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository('path/to/git2r')
##' }
##'
repository <- function(path) {
    ## Argument checking
    stopifnot(is.character(path),
              identical(length(path), 1L),
              nchar(path) > 0)

    path <- normalizePath(path, winslash = "/", mustWork = TRUE)
    if(!file.info(path)$isdir)
        stop('path is not a directory')

    .Call('repository', path)
}

##' Check if repository is bare
##'
##' @name is.bare-methods
##' @aliases is.bare
##' @aliases is.bare-methods
##' @aliases is.bare,repository-method
##' @docType methods
##' @param object The \code{object} to check if it's a bare repository
##' @return TRUE if bare repository, else FALSE
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository('path/to/git2r')
##'
##' ## Check if it's a bare repository
##' is.bare(repo)
##' }
##'
setGeneric('is.bare',
           signature = 'object',
           function(object) standardGeneric('is.bare'))

setMethod('is.bare',
          signature(object = 'repository'),
          function (object)
          {
              .Call('is_bare', object)
          }
)

##' Check if repository is empty
##'
##' @name is.empty-methods
##' @aliases is.empty
##' @aliases is.empty-methods
##' @aliases is.empty,repository-method
##' @docType methods
##' @param object The \code{object} to check if it's a empty repository
##' @return TRUE or FALSE
##' @keywords methods
##' @export
##' @examples
##' \dontrun{
##' ## Open an existing repository
##' repo <- repository('path/to/git2r')
##'
##' ## Check if it's an empty repository
##' is.empty(repo)
##' }
##'
setGeneric('is.empty',
           signature = 'object',
           function(object) standardGeneric('is.empty'))

setMethod('is.empty',
          signature(object = 'repository'),
          function (object)
          {
              .Call('is_empty', object)
          }
)

##' Get the configured remotes for a repo
##'
##' @name remotes-methods
##' @aliases remotes
##' @aliases remotes-methods
##' @aliases remotes,repository-method
##' @docType methods
##' @param object The repository \code{object} to check remotes
##' @return Character vector with remotes
##' @keywords methods
##' @export
setGeneric('remotes',
           signature = 'object',
           function(object) standardGeneric('remotes'))

setMethod('remotes',
          signature(object = 'repository'),
          function (object)
          {
              .Call('remotes', object)
          }
)

##' Get the remote url for remotes in a repo
##'
##' @name remote_url-methods
##' @aliases remote_url
##' @aliases remote_url-methods
##' @aliases remote_url,repository-method
##' @docType methods
##' @param object The repository \code{object} to check remote_url
##' @return Character vector with remote_url
##' @keywords methods
##' @export
setGeneric('remote_url',
           signature = 'object',
           function(object, remote = remotes(object)) standardGeneric('remote_url'))

setMethod('remote_url',
          signature(object = 'repository'),
          function (object, remote)
          {
              .Call('remote_url', object, remote)
          }
)

setMethod('show',
          signature(object = 'repository'),
          function(object)
          {
              lapply(remotes(object), function(remote) {
                  cat(sprintf('Remote:   @ %s (%s)\n', remote, remote_url(repo, remote)))
              })

              if(is.empty(object)) {
                  cat(sprintf('Local:    %s\n', workdir(object)))
                  cat('Head:     nothing commited (yet)\n')
              } else {
                  cat(sprintf('Local:    %s %s\n',
                              head(object)@shorthand,
                              workdir(object)))
              }
          }
)

setMethod('summary',
          signature(object = 'repository'),
          function(object, ...)
          {
              show(object)
              cat('\n')

              ## Branches
              n <-  sum(!is.na(unique(sapply(branches(object), slot, 'hex'))))
              cat(sprintf('Number of branches: %i\n', n))

              ## Tags
              n <-  sum(!is.na(unique(sapply(tags(object), slot, 'hex'))))
              cat(sprintf('Number of tags:     %i\n', n))
          }
)

##' Workdir of repository
##'
##' @name workdir-methods
##' @aliases workdir
##' @aliases workdir-methods
##' @aliases workdir,repository-method
##' @docType methods
##' @param object The repository \code{object} to check workdir
##' @return Character vector with workdir
##' @keywords methods
##' @export
setGeneric('workdir',
           signature = 'object',
           function(object) standardGeneric('workdir'))

setMethod('workdir',
          signature(object = 'repository'),
          function (object)
          {
              .Call('workdir', object)
          }
)
