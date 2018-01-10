## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013 - 2018 The git2r contributors
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

##' Create a new environmental credential object
##'
##' Environmental variables can be written to the file
##' \code{.Renviron}. This file is read by \emph{R} during startup,
##' see \code{\link[base]{Startup}}.
##' @rdname cred_env-methods
##' @family git credential functions
##' @param username The name of the environmental variable that holds
##' the username for the authentication.
##' @param password The name of the environmental variable that holds
##' the password for the authentication.
##' @return A S4 \code{cred_env} object
##' @keywords methods
##' @examples
##' \dontrun{
##' ## Create an environmental credential object for the username and
##' ## password.
##' cred <- cred_env("NAME_OF_ENV_VARIABLE_WITH_USERNAME",
##'                  "NAME_OF_ENV_VARIABLE_WITH_PASSWORD")
##' repo <- repository("git2r")
##' push(repo, credentials = cred)
##' }
setGeneric("cred_env",
           signature = c("username", "password"),
           function(username, password)
           standardGeneric("cred_env"))

##' @rdname cred_env-methods
##' @export
setMethod("cred_env",
          signature(username = "character",
                    password = "character"),
          function(username, password)
          {
              new("cred_env",
                  username = username,
                  password = password)
          }
)

##' Create a new personal access token credential object
##'
##' The personal access token is stored in an envrionmental variable.
##' Environmental variables can be written to the file
##' \code{.Renviron}. This file is read by \emph{R} during startup,
##' see \code{\link[base]{Startup}}. On GitHub, personal access tokens
##' function like ordinary OAuth access tokens. They can be used
##' instead of a password for Git over HTTPS, see
##' \url{https://help.github.com/articles/creating-an-access-token-for-command-line-use/}
##' @family git credential functions
##' @param token The name of the environmental variable that holds the
##' personal access token for the authentication. Defualt is
##' \code{GITHUB_PAT}.
##' @return A S4 \code{cred_token} object
##' @export
##' @examples
##' \dontrun{
##' ## Create a personal access token credential object.
##' ## This example assumes that the token is stored in
##' ## the 'GITHUB_PAT' environmental variable.
##' repo <- repository("git2r")
##' cred <- cred_token()
##' push(repo, credentials = cred)
##' }
cred_token <- function(token = "GITHUB_PAT")
{
    new("cred_token", token = token)
}

##' Create a new plain-text username and password credential object
##'
##' @rdname cred_user_pass-methods
##' @family git credential functions
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
##' @family git credential functions
##' @param publickey The path to the public key of the
##' credential. Default is \code{'~/.ssh/id_rsa.pub'}
##' @param privatekey The path to the private key of the
##' credential. Default is \code{'~/.ssh/id_rsa'}
##' @param passphrase The passphrase of the credential. Default is
##' \code{character(0)}. If getPass is installed and private key is
##' passphrase protected \code{getPass::getPass()} will be called
##' to allow for interactive and obfuscated interactive input of
##' the passphrase.
##' @return A S4 \code{cred_ssh_key} object
##' @export
##' @examples
##' \dontrun{
##' ## Create a ssh key credential object. It can optionally be
##' ## passphrase-protected
##' cred <- cred_ssh_key("~/.ssh/id_rsa.pub", "~/.ssh/id_rsa")
##' repo <- repository("git2r")
##' push(repo, credentials = cred)
##' }
cred_ssh_key <-  function (publickey = "~/.ssh/id_rsa.pub",
                           privatekey = "~/.ssh/id_rsa",
                           passphrase = character(0))
{
    publickey = normalizePath(publickey, mustWork = TRUE)
    privatekey = normalizePath(privatekey, mustWork = TRUE)

    if (length(passphrase) == 0) {
        if (ssh_key_needs_passphrase(privatekey)) {
            if (requireNamespace("getPass", quietly = TRUE)) {
                passphrase <- getPass::getPass()
            }
        }
    }

    new("cred_ssh_key",
        publickey  = publickey,
        privatekey = privatekey,
        passphrase = passphrase)
}

##' Check if private key is passphrase protected
##' @param privatekey The path to the private key of the
##' credential. Default is \code{'~/.ssh/id_rsa'}
##' @noRd
ssh_key_needs_passphrase <- function(privatekey = "~/.ssh/id_rsa"){
  private_content    <- readLines(privatekey, n=3)
  contains_encrypted <- grepl("encrypted",  private_content , ignore.case = TRUE)
  return(
    any(contains_encrypted)
  )
}
