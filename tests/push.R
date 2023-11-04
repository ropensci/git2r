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
path_repo <- tempfile(pattern = "git2r-")

dir.create(path_bare)
dir.create(path_repo)

## Create repositories
bare_repo <- init(path_bare, bare = TRUE)
repo <- clone(path_bare, path_repo)

## Check the repositores
stopifnot(identical(is_bare(bare_repo), TRUE))
stopifnot(identical(is_bare(repo), FALSE))

## Config repositories
config(repo, user.name = "Alice", user.email = "alice@example.org")

## Add changes to repo
writeLines("Hello world", con = file.path(path_repo, "test.txt"))
add(repo, "test.txt")
commit_1 <- commit(repo, "Commit message")
branch_name <- branches(repo)[[1]]$name

## Check commit
stopifnot(identical(commit_1$author$name, "Alice"))
stopifnot(identical(commit_1$author$email, "alice@example.org"))
stopifnot(identical(length(commits(repo)), 1L))
stopifnot(identical(commits(repo)[[1]]$author$name, "Alice"))
stopifnot(identical(commits(repo)[[1]]$author$email, "alice@example.org"))

## Check push arguments
tools::assertError(push(repo, character(0), paste0("refs/heads/", branch_name)))
tools::assertError(push(repo, NA_character_, paste0("refs/heads/", branch_name)))
tools::assertError(push(repo, c("origin", "origin"), paste0("refs/heads/", branch_name)))
tools::assertError(push(repo, "origin"))
tools::assertError(push(repo, name = "origin"))
push(repo, "origin", character(0))
push(repo, "origin", NA_character_)
push(repo, "origin", c(NA_character_, NA_character_))
stopifnot(identical(reflog(repo, paste0("refs/remotes/origin/", branch_name)),
                    structure(list(), class = "git_reflog")))

## No tracking branch assigned to master
tools::assertError(push(branches(repo)[[1]]))

## Push changes from repo to origin
push(repo, "origin", paste0("refs/heads/", branch_name))
r <- reflog(repo, paste0("refs/remotes/origin/", branch_name))
stopifnot(identical(length(r), 1L))
r <- r[[1]]
stopifnot(identical(sha(r), sha(commit_1)))
stopifnot(identical(r$message, "update by push"))
stopifnot(identical(r$index, 0L))
stopifnot(identical(r$committer$name, "Alice"))
stopifnot(identical(r$committer$email, "alice@example.org"))
stopifnot(identical(r$refname, paste0("refs/remotes/origin/", branch_name)))
stopifnot(identical(r$repo$path, repo$path))
push(branches(repo)[[1]])

## Check result in bare repository
stopifnot(identical(length(commits(bare_repo)), 1L))
bare_commit_1 <- commits(bare_repo)[[1]]
stopifnot(identical(sha(commit_1), sha(bare_commit_1)))
stopifnot(identical(commit_1$author, bare_commit_1$author))
stopifnot(identical(commit_1$committer, bare_commit_1$committer))
stopifnot(identical(commit_1$summary, bare_commit_1$summary))
stopifnot(identical(commit_1$message, bare_commit_1$message))
stopifnot(!identical(commit_1$repo, bare_commit_1$repo))

## Add changes to repo and push head
writeLines(c("Hello world", "HELLO WORLD"),
           con = file.path(path_repo, "test.txt"))
add(repo, "test.txt")
commit_2 <- commit(repo, "Commit message 2")
push(repo)
bare_commit_2 <- lookup(bare_repo, sha(commit_2))
stopifnot(identical(sha(commit_2), sha(bare_commit_2)))
stopifnot(identical(commit_2$author, bare_commit_2$author))
stopifnot(identical(commit_2$committer, bare_commit_2$committer))
stopifnot(identical(commit_2$summary, bare_commit_2$summary))
stopifnot(identical(commit_2$message, bare_commit_2$message))
stopifnot(!identical(commit_2$repo, bare_commit_2$repo))

## Check 'set_upstream'
branch_set_upstream(repository_head(repo), NULL)
push(repo, "origin", paste0("refs/heads/", branch_name))
stopifnot(is.null(branch_get_upstream(repository_head(repo))))
push(repo, "origin", paste0("refs/heads/", branch_name), set_upstream = TRUE)
stopifnot(!is.null(branch_get_upstream(repository_head(repo))))

## Cleanup
unlink(path_bare, recursive = TRUE)
unlink(path_repo, recursive = TRUE)
