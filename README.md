[![R-CMD-check](https://github.com/ropensci/git2r/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/ropensci/git2r/actions/workflows/R-CMD-check.yaml)
[![CRAN status](http://www.r-pkg.org/badges/version/git2r)](http://cran.r-project.org/web/packages/git2r/index.html)
[![CRAN RStudio mirror downloads](http://cranlogs.r-pkg.org/badges/last-month/git2r)](http://cran.r-project.org/web/packages/git2r/index.html)
[![Coverage Status](https://coveralls.io/repos/github/ropensci/git2r/badge.svg?branch=master)](https://coveralls.io/github/ropensci/git2r?branch=master)

# Introduction

The `git2r` package gives you programmatic access to Git repositories
from R. Internally the package uses the libgit2 library which is a
pure C implementation of the Git core methods. For more information
about libgit2, check out libgit2's website
[(http://libgit2.github.com)](http://libgit2.github.com).

Suggestions, bugs, forks and pull requests are appreciated. Get in
touch.

## Installation

To install the version available on CRAN:

```coffee
install.packages("git2r")
```

To install the development version of `git2r`, it's easiest to use the
devtools package:

```coffee
# install.packages("devtools")
library(devtools)
install_github("ropensci/git2r")
```

Another alternative is to use `git` and `make`

```coffee
$ git clone https://github.com/ropensci/git2r.git
$ cd git2r
$ make install
```

## Usage

### Repository

The central object in the `git2r` package is the S3 class
`git_repository`. The following three methods can instantiate a
repository; `init`, `repository` and `clone`.

#### Create a new repository

Create a new repository in a temporary directory using `init`

```coffee
library(git2r)
```

```
#> Loading required package: methods
```

```coffee

## Create a temporary directory to hold the repository
path <- tempfile(pattern="git2r-")
dir.create(path)

## Initialize the repository
repo <- init(path)

## Display a brief summary of the new repository
repo
```

```
#> Local:    /tmp/Rtmp7CXPlx/git2r-1ae2305c0e8d/
#> Head:     nothing commited (yet)
```

```coffee

## Check if repository is bare
is_bare(repo)
```

```
#> [1] FALSE
```

```coffee

## Check if repository is empty
is_empty(repo)
```

```
#> [1] TRUE
```

#### Create a new bare repository

```coffee
## Create a temporary directory to hold the repository
path <- tempfile(pattern="git2r-")
dir.create(path)

## Initialize the repository
repo <- init(path, bare=TRUE)

## Check if repository is bare
is_bare(repo)
```

```
#> [1] TRUE
```

#### Clone a repository

```coffee
## Create a temporary directory to hold the repository
path <- file.path(tempfile(pattern="git2r-"), "git2r")
dir.create(path, recursive=TRUE)

## Clone the git2r repository
repo <- clone("https://github.com/ropensci/git2r", path)
```

```
#> cloning into '/tmp/Rtmp7CXPlx/git2r-1ae27d811539/git2r'...
#> Receiving objects:   1% (24/2329),   12 kb
#> Receiving objects:  11% (257/2329),   60 kb
#> Receiving objects:  21% (490/2329),  100 kb
#> Receiving objects:  31% (722/2329),  125 kb
#> Receiving objects:  41% (955/2329),  237 kb
#> Receiving objects:  51% (1188/2329),  574 kb
#> Receiving objects:  61% (1421/2329), 1014 kb
#> Receiving objects:  71% (1654/2329), 1350 kb
#> Receiving objects:  81% (1887/2329), 1733 kb
#> Receiving objects:  91% (2120/2329), 2614 kb
#> Receiving objects: 100% (2329/2329), 2641 kb, done.
```

```coffee

## Summary of repository
summary(repo)
```

```
#> Remote:   @ origin (https://github.com/ropensci/git2r)
#> Local:    master /tmp/Rtmp7CXPlx/git2r-1ae27d811539/git2r/
#>
#> Branches:          1
#> Tags:              0
#> Commits:         320
#> Contributors:      3
#> Ignored files:     0
#> Untracked files:   0
#> Unstaged files:    0
#> Staged files:      0
```

```coffee

## List all references in repository
references(repo)
```

```
#> $`refs/heads/master`
#> [6fb440] master
#>
#> $`refs/remotes/origin/master`
#> [6fb440] origin/master
```

```coffee

## List all branches in repository
branches(repo)
```

```
#> [[1]]
#> [6fb440] (Local) (HEAD) master
#>
#> [[2]]
#> [6fb440] (origin @ https://github.com/ropensci/git2r) master
```

#### Open an existing repository

```coffee
## Open an existing repository
repo <- repository(path)

## Workdir of repository
workdir(repo)
```

```
#> [1] "/tmp/Rtmp7CXPlx/git2r-1ae27d811539/git2r/"
```

```coffee

## List all commits in repository
commits(repo)[[1]] # Truncated here for readability
```

```
#> Commit:  6fb440133765e80649de8d714eaea17b114bd0a7
#> Author:  Stefan Widgren <stefan.widgren@gmail.com>
#> When:    2014-04-22 21:43:19
#> Summary: Fixed clone progress to end line with newline
```

```coffee

## Get HEAD of repository
repository_head(repo)
```

```
#> [6fb440] (Local) (HEAD) master
```

```coffee

## Check if HEAD is head
is_head(repository_head(repo))
```

```
#> [1] TRUE
```

```coffee

## Check if HEAD is local
is_local(repository_head(repo))
```

```
#> [1] TRUE
```

```coffee

## List all tags in repository
tags(repo)
```

```
#> list()
```

### Configuration

```coffee
config(repo, user.name="Git2r Readme", user.email="git2r.readme@example.org")

## Display configuration
config(repo)
```

```
#> global:
#>         core.autocrlf=input
#> local:
#>         branch.master.merge=refs/heads/master
#>         branch.master.remote=origin
#>         core.bare=false
#>         core.filemode=true
#>         core.logallrefupdates=true
#>         core.repositoryformatversion=0
#>         remote.origin.fetch=+refs/heads/*:refs/remotes/origin/*
#>         remote.origin.url=https://github.com/ropensci/git2r
#>         user.email=git2r.readme@example.org
#>         user.name=Git2r Readme
```

### Commit

```coffee
## Create a new file
writeLines("Hello world!", file.path(path, "test.txt"))

## Add file and commit
add(repo, "test.txt")
commit(repo, "Commit message")
```

```
#> Commit:  0a6af48cedf43208bde34230662280514e0956eb
#> Author:  Git2r Readme <git2r.readme@example.org>
#> When:    2014-04-22 21:44:57
#> Summary: Commit message
```

# Included software

- The C library [libgit2](https://github.com/libgit2/libgit2). See
  `inst/AUTHORS` for the authors of libgit2.

- The libgit2 library has been modified, e.g. to use the R printing
  and error routines, and to use `runif` instead of `rand`.

# License

The `git2r` package is licensed under the GPLv2. See these files for additional details:

- LICENSE      - `git2r` package license (GPLv2)
- inst/COPYING - Copyright notices for additional included software

---

[![](http://ropensci.org/public_images/github_footer.png)](http://ropensci.org)
