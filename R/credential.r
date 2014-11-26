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

##' Create a new plain-text username and password credential object
##'
##' @rdname cred_user_pass-methods
##' @param username The username of the credential
##' @param password The password of the credential
##' @return A S4 \code{cred_user_pass} object
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Create a plain-text username and password credential object
##' cred_user_pass("Random Developer", "SecretPassword")
##' }
setGeneric("cred_user_pass",
           signature = c("username", "password"),
           function(username, password)
           standardGeneric("cred_user_pass"))

##' @rdname cred_user_pass-methods
##' @export
setMethod("cred_user_pass",
          signature(username = "character",
                    password = "character"),
          function(username, password)
          {
              new("cred_user_pass",
                  username = username,
                  password = password)
          }
)

##' Create a new passphrase-protected ssh key credential object
##'
##' @rdname cred_ssh_key-methods
##' @param publickey The path to the public key of the credential
##' @param privatekey The path to the private key of the credential
##' @param passphrase The passphrase of the credential
##' @return A S4 \code{cred_ssh_key} object
##' @keywords methods
setGeneric("cred_ssh_key",
           signature = c("publickey", "privatekey", "passphrase"),
           function(publickey, privatekey, passphrase)
           standardGeneric("cred_ssh_key"))

##' @rdname cred_ssh_key-methods
##' @export
setMethod("cred_ssh_key",
          signature(publickey  = "character",
                    privatekey = "character",
                    passphrase = "character"),
          function(publickey, privatekey, passphrase)
          {
              new("cred_ssh_key",
                  publickey  = normalizePath(publickey, mustWork = TRUE),
                  privatekey = normalizePath(privatekey, mustWork = TRUE),
                  passphrase = passphrase)
          }
)

##' @rdname cred_ssh_key-methods
##' @export
setMethod("cred_ssh_key",
          signature(publickey  = "character",
                    privatekey = "character",
                    passphrase = "missing"),
          function(publickey, privatekey, passphrase)
          {
              cred_ssh_key(publickey  = publickey,
                           privatekey = privatekey,
                           passphrase = character(0))
          }
)
