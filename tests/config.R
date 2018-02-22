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

library("git2r")

## For debugging
sessionInfo()

## Create a directory in tempdir
path <- tempfile(pattern="git2r-")
dir.create(path)

## Initialize a repository
repo <- init(path)

## Config repository
cfg <- config(repo, user.name="Alice", user.email="alice@example.org")

## Check configuration
stopifnot("local" %in% names(cfg))
stopifnot("user.name" %in% names(cfg$local))
stopifnot(identical(cfg$local$user.name, "Alice"))
stopifnot(identical(cfg$local$user.email, "alice@example.org"))

## Delete entries
cfg <- config(repo, user.name=NULL, user.email=NULL)

## Check configuration
stopifnot(is.null(cfg$local$user.name))
stopifnot(is.null(cfg$local$user.email))

## Supply values as objects
user.name <- "Alice"
user.email <- "alice@example.org"
cfg <- config(repo, user.name=user.name, user.email="alice@example.org")
stopifnot(identical(cfg$local$user.name, user.name))
stopifnot(identical(cfg$local$user.email, "alice@example.org"))
cfg <- config(repo, user.name="Alice", user.email=user.email)
stopifnot(identical(cfg$local$user.name, "Alice"))
stopifnot(identical(cfg$local$user.email, user.email))

## Check git config files
cfg <- git_config_files(repo)
stopifnot(identical(dim(cfg), c(4L, 2L)))
stopifnot(identical(cfg$file, c("system", "xdg", "global", "local")))
stopifnot(!is.na(cfg$path[4]))

## Check location of .gitconfig on Windows
if (identical(Sys.getenv("APPVEYOR"), "True")) {

  ## AppVeyor diagnostics
  str(Sys.getenv("USERPROFILE"))
  str(Sys.getenv("HOMEDRIVE"))
  str(normalizePath("~"))
  str(git_config_files())

  ## Temporarily move AppVeyor .gitconfig
  gitconfig_appveyor <- "C:/Users/appveyor/.gitconfig"
  gitconfig_tmp <- "/tmp/.gitconfig"
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

  ## Return AppVeyor .gitconfig
  file.rename(gitconfig_tmp, gitconfig_appveyor)
}

## Cleanup
unlink(path, recursive=TRUE)
