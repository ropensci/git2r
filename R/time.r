#' @include branch.r
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
##' @docType class
##' @keywords classes
##' @section Methods:
##' \describe{
##'   \item{show}{\code{signature(object = "git_time")}}
##' }
##' @keywords methods
##' @export
setClass('git_time',
         slots=c(time='numeric',
                 offset='numeric'))

setAs(from='git_time',
      to='character',
      def=function(from)
      {
          as.character(as(from, 'POSIXct'))
      }
)

setAs(from='git_time',
      to='POSIXct',
      def=function(from)
      {
          as.POSIXct(from@time + from@offset*60,
                     origin='1970-01-01',
                     tz='GMT')
      }
)

setMethod('show',
          signature(object = 'git_time'),
          function(object)
          {
              cat(sprintf('%s\n', as(object, 'character')))
          }
)
