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

##' Class \code{"cred_plaintext"}
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
##' @rdname cred_plaintext-class
##' @docType class
##' @keywords classes
##' @keywords methods
##' @export
setClass("cred_plaintext",
         slots=c(username = "character",
                 password = "character")
)

##' Create a new plain-text username and password credential object
##'
##' @param username The username of the credential
##' @param password The password of the credential
##' @return A S4 \code{cred_plaintext} object
##' @keywords methods
##' @export
cred_plaintext <- function(username, password) {
    new("cred_plaintext", username = username, password = password)
}
