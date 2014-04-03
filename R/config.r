## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2014  Stefan Widgren
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

##' Config
##'
##' Config file management.
##' @rdname config-methods
##' @docType methods
##' @param repo the \code{repo} to configure
##' @param user.name the user name
##' @param user.email the e-mail address
##' @return A \code{list} with the configuration
##' @keywords methods
##' @include repository.r
##' @examples \dontrun{
##' ## Open an existing repository
##' repo <- repository('path/to/git2r')
##'
##' config(repo, user.name='Stefan Widgren', user.email='stefan.widgren@@gmail.com')
##'}
setGeneric('config',
           signature = 'repo',
           function(repo,
                    user.name,
                    user.email)
           standardGeneric('config'))

##' @rdname config-methods
##' @export
setMethod('config',
          signature(repo = 'git_repository'),
          function(repo,
                   user.name,
                   user.email)
          {
              variables <- as.list(match.call(expand.dots = TRUE)[c(-1, -2)])

              check_is_character <- sapply(variables, is.character)
              check_is_character <- check_is_character[!check_is_character]
              if(length(check_is_character)) {
                  stop(sprintf('\n%s', paste(names(check_is_character),
                                             'must be character',
                                             collapse='\n')))
              }

              .Call('config', repo, variables)
          }
)
