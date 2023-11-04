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

library(git2r)

## For debugging
sessionInfo()
libgit2_version()
libgit2_features()


## Create 2 directories in tempdir
path_bare <- tempfile(pattern = "git2r-")
path_repo_1 <- tempfile(pattern = "git2r-")
path_repo_2 <- tempfile(pattern = "git2r-")

dir.create(path_bare)
dir.create(path_repo_1)
dir.create(path_repo_2)

## Create repositories
bare_repo <- init(path_bare, bare = TRUE)
repo_1 <- clone(path_bare, path_repo_1)
repo_2 <- clone(path_bare, path_repo_2)

## Config repositories
config(repo_1, user.name = "Alice", user.email = "alice@example.org")
config(repo_2, user.name = "Bob", user.email = "bob@example.org")

## Add changes to repo 1
writeLines("Hello world", con = file.path(path_repo_1, "test.txt"))
add(repo_1, "test.txt")
commit_1 <- commit(repo_1, "Commit message")
branch_name <- branches(repo_1)[[1]]$name

## Push changes from repo 1 to origin
push(repo_1, "origin", paste0("refs/heads/", branch_name))

## Check result in bare repository
stopifnot(identical(length(commits(bare_repo)), 1L))
bare_commit_1 <- commits(bare_repo)[[1]]
stopifnot(identical(sha(commit_1), sha(bare_commit_1)))
stopifnot(identical(commit_1$author, bare_commit_1$author))
stopifnot(identical(commit_1$committer, bare_commit_1$committer))
stopifnot(identical(commit_1$summary, bare_commit_1$summary))
stopifnot(identical(commit_1$message, bare_commit_1$message))
stopifnot(!identical(commit_1$repo, bare_commit_1$repo))

## Fetch
fetch(repo_2, "origin")
fh <- fetch_heads(repo_2)[[1]]
stopifnot(identical(sha(fh), fh$sha))

## Test show method of non-empty repository where head is null
show(repo_2)

## Check that 'git2r_arg_check_credentials' raise error
res <- tools::assertError(
                  .Call(git2r:::git2r_remote_fetch, repo_1, "origin",
                        3, "fetch", FALSE, NULL))
stopifnot(length(grep("'credentials' must be an S3 class with credentials",
                      res[[1]]$message)) > 0)

res <- tools::assertError(
    .Call(git2r:::git2r_remote_fetch, repo_1, "origin", repo_1,
          "fetch", FALSE, NULL))
stopifnot(length(grep("'credentials' must be an S3 class with credentials",
                      res[[1]]$message)) > 0)

credentials <- cred_env(c("username", "username"), "password")
res <- tools::assertError(
    .Call(git2r:::git2r_remote_fetch, repo_1, "origin", credentials,
          "fetch", FALSE, NULL))
stopifnot(length(grep("'credentials' must be an S3 class with credentials",
                      res[[1]]$message)) > 0)

credentials <- cred_env("username", c("password", "passowrd"))
res <- tools::assertError(
    .Call(git2r:::git2r_remote_fetch, repo_1, "origin", credentials,
          "fetch", FALSE, NULL))
stopifnot(length(grep("'credentials' must be an S3 class with credentials",
                      res[[1]]$message)) > 0)

credentials <- cred_user_pass(c("username", "username"), "password")
res <- tools::assertError(
    .Call(git2r:::git2r_remote_fetch, repo_1, "origin", credentials,
          "fetch", FALSE, NULL))
stopifnot(length(grep("'credentials' must be an S3 class with credentials",
                      res[[1]]$message)) > 0)

credentials <- cred_user_pass("username", c("password", "passowrd"))
res <- tools::assertError(
    .Call(git2r:::git2r_remote_fetch, repo_1, "origin", credentials,
          "fetch", FALSE, NULL))
stopifnot(length(grep("'credentials' must be an S3 class with credentials",
                      res[[1]]$message)) > 0)

credentials <- cred_token(c("GITHUB_PAT", "GITHUB_PAT"))
res <- tools::assertError(
    .Call(git2r:::git2r_remote_fetch, repo_1, "origin", credentials,
          "fetch", FALSE, NULL))
stopifnot(length(grep("'credentials' must be an S3 class with credentials",
                      res[[1]]$message)) > 0)

credentials <- structure(list(publickey  = c("id_rsa.pub", "id_rsa.pub"),
                              privatekey = "id_rsa",
                              passphrase = character(0)),
                         class = "cred_ssh_key")
res <- tools::assertError(
    .Call(git2r:::git2r_remote_fetch, repo_1, "origin", credentials,
          "fetch", FALSE, NULL))
stopifnot(length(grep("'credentials' must be an S3 class with credentials",
                      res[[1]]$message)) > 0)

credentials <- structure(list(publickey  = "id_rsa.pub",
                              privatekey = c("id_rsa", "id_rsa"),
                              passphrase = character(0)),
                         class = "cred_ssh_key")
res <- tools::assertError(
    .Call(git2r:::git2r_remote_fetch, repo_1, "origin", credentials,
          "fetch", FALSE, NULL))
stopifnot(length(grep("'credentials' must be an S3 class with credentials",
                      res[[1]]$message)) > 0)

credentials <- structure(list(publickey  = "id_rsa.pub",
                              privatekey = "id_rsa",
                              passphrase = NA_character_),
                         class = "cred_ssh_key")
res <- tools::assertError(
    .Call(git2r:::git2r_remote_fetch, repo_1, "origin", credentials,
          "fetch", FALSE, NULL))
stopifnot(length(grep("'credentials' must be an S3 class with credentials",
                      res[[1]]$message)) > 0)

credentials <- structure(list(publickey  = "id_rsa.pub",
                              privatekey = "id_rsa",
                              passphrase = c("passphrase", "passphrase")),
                         class = "cred_ssh_key")
res <- tools::assertError(
    .Call(git2r:::git2r_remote_fetch, repo_1, "origin", credentials,
          "fetch", FALSE, NULL))
stopifnot(length(grep("'credentials' must be an S3 class with credentials",
                      res[[1]]$message)) > 0)

## Cleanup
unlink(path_bare, recursive = TRUE)
unlink(path_repo_1, recursive = TRUE)
unlink(path_repo_2, recursive = TRUE)
