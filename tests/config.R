## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2023 The git2r contributors
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

library("git2r")

## For debugging
sessionInfo()
libgit2_version()
libgit2_features()


## Create a directory in tempdir
path <- tempfile(pattern = "git2r-")
dir.create(path)

## Initialize a repository
repo <- init(path)

## Config repository
cfg <- config(repo, user.name = "Alice", user.email = "alice@example.org")

## Check configuration
stopifnot(identical(print(cfg), cfg))
stopifnot("local" %in% names(cfg))
stopifnot("user.name" %in% names(cfg$local))
stopifnot(identical(cfg$local$user.name, "Alice"))
stopifnot(identical(cfg$local$user.email, "alice@example.org"))

## Check that config fails for non-character entry.
tools::assertError(config(repo, test = 5))

## Check config method with missing repo argument
wd <- setwd(path)
cfg <- config(user.name = "Alice", user.email = "alice@example.org")
stopifnot("local" %in% names(cfg))
stopifnot("user.name" %in% names(cfg$local))
stopifnot(identical(cfg$local$user.name, "Alice"))
stopifnot(identical(cfg$local$user.email, "alice@example.org"))
stopifnot(identical(git_config_files(repo = repo)$local,
                    git_config_files(repo = NULL)$local))
stopifnot(identical(git_config_files(repo = repo)$local,
                    git_config_files(repo = repo$path)$local))
if (!is.null(wd))
    setwd(wd)

## Delete entries
cfg <- config(repo, user.name = NULL, user.email = NULL)

## Check configuration
stopifnot(is.null(cfg$local$user.name))
stopifnot(is.null(cfg$local$user.email))

## Supply values as objects
user_name <- "Alice"
user_email <- "alice@example.org"
cfg <- config(repo, user.name = user_name, user.email = "alice@example.org")
stopifnot(identical(cfg$local$user.name, user_name))
stopifnot(identical(cfg$local$user.email, "alice@example.org"))
cfg <- config(repo, user.name = "Alice", user.email = user_email)
stopifnot(identical(cfg$local$user.name, "Alice"))
stopifnot(identical(cfg$local$user.email, user_email))

## Check git config files
cfg <- git_config_files(repo)
stopifnot(identical(nrow(cfg), 4L))
stopifnot(identical(names(cfg), c("file", "path")))
stopifnot(identical(cfg$file, c("system", "xdg", "global", "local")))
stopifnot(!is.na(cfg$path[4]))

## Check that the local config file is NA for an invalid repo
## argument.
stopifnot(is.na(git_config_files(5)$local))

## Check location of .gitconfig on Windows
if (identical(Sys.getenv("APPVEYOR"), "True")) {

  ## AppVeyor diagnostics
  str(Sys.getenv("USERPROFILE"))
  str(Sys.getenv("HOMEDRIVE"))
  str(normalizePath("~"))
  str(git_config_files())

  ## Temporarily move AppVeyor .gitconfig
  gitconfig_appveyor <- "C:/Users/appveyor/.gitconfig"
  gitconfig_tmp <- file.path(tempdir(), ".gitconfig")
  file.rename(gitconfig_appveyor, gitconfig_tmp)

  ## Test config() on Windows
  gitconfig_expected <- file.path(Sys.getenv("USERPROFILE"), ".gitconfig")
  ## .gitconfig should not be created if no configuration options specified
  config(global = TRUE)
  stopifnot(!file.exists(gitconfig_expected))
  ## .gitconfig should be created in the user's home directory
  config(global = TRUE, user.name = "name", user.email = "email")
  stopifnot(file.exists(gitconfig_expected))
  unlink(gitconfig_expected)
  ## .gitconfig should be created if user specifies option other than user.name
  ## and user.email
  config(global = TRUE, core.editor = "nano")
  stopifnot(file.exists(gitconfig_expected))
  unlink(gitconfig_expected)
  ## .gitconfig should not create a new .gitconfig if the user already has one
  ## in Documents/
  gitconfig_documents <- "~/.gitconfig"
  file.create(gitconfig_documents)
  config(global = TRUE, core.editor = "nano")
  stopifnot(!file.exists(gitconfig_expected))
  unlink(gitconfig_documents)

  ## Return AppVeyor .gitconfig
  file.rename(gitconfig_tmp, gitconfig_appveyor)
}

## Cleanup
unlink(path, recursive = TRUE)
