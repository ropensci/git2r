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
##' @docType class
##' @keywords classes
##' @keywords methods
##' @export
setClass("cred_ssh_key",
         slots=c(publickey  = "character",
                 privatekey = "character",
                 passphrase = "character")
)

##' Create a new passphrase-protected ssh key credential object
##'
##' @param publickey The path to the public key of the credential
##' @param privatekey The path to the private key of the credential
##' @param passphrase The passphrase of the credential
##' @return A S4 \code{cred_ssh_key} object
##' @keywords methods
##' @export
cred_ssh_key <- function(publickey, privatekey, passphrase) {
    new("cred_ssh_key",
        publickey  = publickey,
        privatekey = privatekey,
        passphrase = passphrase)
}
