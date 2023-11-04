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
config(repo, user.name = "Alice")

## Let's set one valid and one with variable with invalid format
res <- tools::assertWarning(config(repo,
                                   user.email = "alice@example.org",
                                   lol = "wut"))
stopifnot(length(grep("Variable was not in a valid format: 'lol'",
                      res[[1]]$message)) > 0)

cfg_exp <- structure(list(user.name = "Alice",
                          user.email = "alice@example.org",
                          "NA" = NULL),
                     .Names = c("user.name", "user.email", NA))

cfg_obs <- config(repo)$local
cfg_obs <- cfg_obs[c("user.name", "user.email", "lol")]
stopifnot(identical(cfg_obs, cfg_exp))

## Cleanup
unlink(path, recursive = TRUE)
