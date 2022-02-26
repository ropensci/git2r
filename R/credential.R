## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013 - 2022 The git2r contributors
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
##' @family git credential functions
##' @param username The name of the environmental variable that holds
##'     the username for the authentication.
##' @param password The name of the environmental variable that holds
##'     the password for the authentication.
##' @return A list of class \code{cred_env} with entries:
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
##' @export
##' @examples
##' \dontrun{
##' ## Create an environmental credential object for the username and
##' ## password.
##' cred <- cred_env("NAME_OF_ENV_VARIABLE_WITH_USERNAME",
##'                  "NAME_OF_ENV_VARIABLE_WITH_PASSWORD")
##' repo <- repository("git2r")
##' push(repo, credentials = cred)
##' }
cred_env <- function(username = NULL, password = NULL) {
  structure(list(username = username, password = password),
            class = "cred_env")
}

##' Create a new personal access token credential object
##'
##' The personal access token is stored in an envrionmental variable.
##' Environmental variables can be written to the file
##' \code{.Renviron}. This file is read by \emph{R} during startup,
##' see \code{\link[base]{Startup}}. On GitHub, personal access tokens
##' function like ordinary OAuth access tokens. They can be used
##' instead of a password for Git over HTTPS, see the \dQuote{Creating
##' a personal access token} article on GitHub Docs.
##' @family git credential functions
##' @param token The name of the environmental variable that holds the
##'     personal access token for the authentication. Default is
##'     \code{GITHUB_PAT}.
##' @return A list of class \code{cred_token} with entry:
##' \describe{
##'   \item{token}{
##'     The name of the environmental variable that holds
##'     the personal access token for the authentication.
##'   }
##' }
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
cred_token <- function(token = "GITHUB_PAT") {
  structure(list(token = token), class = "cred_token")
}

##' Create a new plain-text username and password credential object
##'
##' @family git credential functions
##' @param username The username of the credential
##' @param password The password of the credential. If getPass is installed
##'     and the only input is username, \code{getPass::getPass()} will be
##'     called to allow for interactive and obfuscated interactive
##'     input of the password.
##' @return A list of class \code{cred_user_pass} with entries:
##' \describe{
##'   \item{username}{
##'     The username of the credential
##'   }
##'   \item{password}{
##'     The password of the credential
##'   }
##' }
##' @export
##' @examples
##' \dontrun{
##' ## Create a plain-text username and password credential object
##' cred_user_pass("Random Developer", "SecretPassword")
##' }
cred_user_pass <- function(username = NULL, password = NULL) {

  if (!is.null(username)) {
    if (is.null(password)) {
      if (requireNamespace("getPass", quietly = TRUE)) {
        password <- getPass::getPass()
      }
    }
  }

  structure(list(username = username, password = password),
            class = "cred_user_pass")
}

##' Create a new passphrase-protected ssh key credential object
##'
##' @family git credential functions
##' @param publickey The path to the public key of the
##'     credential. Default is \code{ssh_path("id_rsa.pub")}
##' @param privatekey The path to the private key of the
##'     credential. Default is \code{ssh_path("id_rsa")}
##' @param passphrase The passphrase of the credential. Default is
##'     \code{character(0)}. If getPass is installed and private key
##'     is passphrase protected \code{getPass::getPass()} will be
##'     called to allow for interactive and obfuscated interactive
##'     input of the passphrase.
##' @return A list of class \code{cred_ssh_key} with entries:
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
##' @export
##' @examples
##' \dontrun{
##' ## Create a ssh key credential object. It can optionally be
##' ## passphrase-protected
##' cred <- cred_ssh_key(ssh_path("id_rsa.pub"), ssh_path("id_rsa"))
##' repo <- repository("git2r")
##' push(repo, credentials = cred)
##' }
cred_ssh_key <- function(publickey = ssh_path("id_rsa.pub"),
                         privatekey = ssh_path("id_rsa"),
                         passphrase = character(0)) {
  publickey <- normalizePath(publickey, mustWork = TRUE)
  privatekey <- normalizePath(privatekey, mustWork = TRUE)

  if (length(passphrase) == 0) {
    if (ssh_key_needs_passphrase(privatekey)) {
      if (requireNamespace("getPass", quietly = TRUE)) {
        passphrase <- getPass::getPass()
      }
    }
  }

  structure(list(publickey  = publickey,
                 privatekey = privatekey,
                 passphrase = passphrase),
            class = "cred_ssh_key")
}

##' Check if private key is passphrase protected
##' @param privatekey The path to the private key of the
##' credential. Default is \code{ssh_path("id_rsa")}
##' @noRd
ssh_key_needs_passphrase <- function(privatekey = ssh_path("id_rsa")) {
  private_content <- readLines(privatekey, n = 3)
  contains_encrypted <- grepl("encrypted", private_content, ignore.case = TRUE)
  any(contains_encrypted)
}

##' Compose usual path to ssh keys
##'
##' This function provides a consistent means across OS-types to access the
##' \code{.ssh} directory.
##'
##' On Windows-based systems,
##' \code{path.expand("~")} returns \code{"C:/Users/username/Documents"},
##' whereas the usual path to the \code{.ssh} directory is
##' \code{"C:/Users/username"}.
##'
##' On other operating systems, \code{path.expand("~")} returns the usual path
##' to the \code{.ssh} directory.
##'
##' Calling \code{ssh_path()} with no arguments will return the usual path to
##' the \code{.ssh} directory.
##'
##' @param file basename of file for which path is requested
##' @return Full path to the file
##' @export
##' @examples
##' ssh_path()
##' ssh_path("is_rsa.pub")
ssh_path <- function(file = "") {
  file.path(home_dir(), ".ssh", file)
}

# Return the user's home directory regardless of operating system
home_dir <- function() {
  if (.Platform$OS.type == "windows") {
    home <- Sys.getenv("USERPROFILE")
  } else {
    home <- path.expand("~")
  }
  return(home)
}
