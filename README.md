[![Build Status](https://travis-ci.org/ropensci/git2r.png)](https://travis-ci.org/ropensci/git2r)

git2r
=====

R bindings to [libgit2](https://github.com/libgit2/libgit2) library. The package uses the source code of `libgit2` to interface a Git repository from R.

Aim
---

The aim of the package is to be able to run some basic git commands on a repository from R. Another aim is to extract and visualize descriptive statistics from a git repository.

Development
-----------

The package is in a very early development phase and is considered unstable with only a few features implemented.

Installation
------------

To install the development version of `git2r`, it's easiest to use the devtools package:






```coffee
# install.packages("devtools")
library(devtools)
install_github("git2r", "ropensci")
```


Example
-------


```coffee
library(git2r)

# Open an existing repository
# repo <- repository("path/to/git2r")
repo <- repository(getwd())

# Brief summary of repository
repo
```

```
#> Remote:   @ origin (https://github.com/ropensci/git2r)
#> Local:    master /Users/karthik/Documents/work/Github/ropensci/git2r/
```

```coffee

# Summary of repository
summary(repo)
```

```
#> Remote:   @ origin (https://github.com/ropensci/git2r)
#> Local:    master /Users/karthik/Documents/work/Github/ropensci/git2r/
#> 
#> Branches:      10
#> Tags:          0
#> Commits:       170
#> Contributors:  3
```

```coffee

# Workdir of repository
workdir(repo)
```

```
#> [1] "/Users/karthik/Documents/work/Github/ropensci/git2r/"
```

```coffee

# Check if repository is bare
is.bare(repo)
```

```
#> [1] FALSE
```

```coffee

# Check if repository is empty
is.empty(repo)
```

```
#> [1] FALSE
```

```coffee

# List all references in repository
references(repo)
```

```
#> $`refs/heads/contrib`
#> [bd5d2c] contrib
#> 
#> $`refs/heads/contrib_fns`
#> [786472] contrib_fns
#> 
#> $`refs/heads/contributions`
#> [5ce142] contributions
#> 
#> $`refs/heads/foo`
#> [a98a5f] foo
#> 
#> $`refs/heads/master`
#> [01bf4a] master
#> 
#> $`refs/heads/roxygen-update`
#> [abaac5] roxygen-update
#> 
#> $`refs/heads/testing_check`
#> [693eb7] testing_check
#> 
#> $`refs/remotes/origin/example_fix`
#> [1dda04] origin/example_fix
#> 
#> $`refs/remotes/origin/HEAD`
#> refs/remotes/origin/HEAD => refs/remotes/origin/master
#> 
#> $`refs/remotes/origin/master`
#> [01bf4a] origin/master
#> 
#> $`refs/remotes/origin/testing_check`
#> [a64614] origin/testing_check
#> 
#> $`refs/remotes/origin/travis_fix`
#> [9089f6] origin/travis_fix
#> 
#> $`refs/stash`
#> [8350c5] stash
```

```coffee

# List all branches in repository
branches(repo)
```

```
#> [[1]]
#> [bd5d2c] (Local) contrib
#> 
#> [[2]]
#> [786472] (Local) contrib_fns
#> 
#> [[3]]
#> [5ce142] (Local) contributions
#> 
#> [[4]]
#> [a98a5f] (Local) foo
#> 
#> [[5]]
#> [01bf4a] (Local) (HEAD) master
#> 
#> [[6]]
#> [abaac5] (Local) roxygen-update
#> 
#> [[7]]
#> [693eb7] (Local) testing_check
#> 
#> [[8]]
#> [1dda04] (origin @ https://github.com/ropensci/git2r) example_fix
#> 
#> [[9]]
#> (origin @ https://github.com/ropensci/git2r) refs/remotes/origin/HEAD => refs/remotes/origin/master
#> 
#> [[10]]
#> [01bf4a] (origin @ https://github.com/ropensci/git2r) master
#> 
#> [[11]]
#> [a64614] (origin @ https://github.com/ropensci/git2r) testing_check
#> 
#> [[12]]
#> [9089f6] (origin @ https://github.com/ropensci/git2r) travis_fix
```

```coffee

# List all commits in repository
commits(repo)[1] # Truncated here for readability
```

```
#> [[1]]
#> Commit:  01bf4ae4436a9bb80bd2cf55782c46fe86943519
#> Author:  Stefan Widgren <stefan.widgren@gmail.com>
#> When:    2014-03-22 17:27:08
#> Summary: Fixed roxygen exportMethod tag in documentation
```

```coffee

# Get HEAD of repository
head(repo)
```

```
#> [01bf4a] (Local) (HEAD) master
```

```coffee

# Check if HEAD is head
is.head(head(repo))
```

```
#> [1] TRUE
```

```coffee

# Check if HEAD is local
is.local(head(repo))
```

```
#> [1] TRUE
```

```coffee

# List all tags in repository
tags(repo)
```

```
#> named list()
```


### Visualize the number of commits per month in a repository


```coffee
library(git2r)
contributions()
```

```
#>         when  n
#> 1 2013-12-01 35
#> 2 2014-01-01 33
#> 3 2014-02-01  7
#> 4 2014-03-01 95
```




### Visualize contributions by user on a monthly timeline (another way of looking at the same data from above)


```coffee
# This needs fixing in the new version
library(git2r)
library(dplyr)
contributions(by = "user")
contributions(breaks = "months", data_only = TRUE, by = "user")
contributions(breaks = "weeks", by = "user")
contributions(breaks = "days", by = "user")

```


### Generate a wordcloud from the commit messages in a repository


```coffee
library(git2r)
library(wordcloud)
```

```
#> Loading required package: Rcpp
#> Loading required package: RColorBrewer
```

```coffee
library(RColorBrewer)

# Open an existing repository
# repo <- repository("path/to/git2r")
repo <- repository(getwd())

## Create the wordcloud
wordcloud(paste(sapply(commits(repo), slot, 'message'), collapse=' '),
          scale=c(5,0.5), max.words = 100, random.order = FALSE,
          rot.per = 0.35, use.r.layout = FALSE,
          colors = brewer.pal(8, 'Dark2'))
```

```
#> Loading required package: tm
```

![plot of chunk wordcloud](figure/wordcloud.png) 



Included software
-----------------

- The C library [libgit2](https://github.com/libgit2/libgit2). See
  `inst/AUTHORS` for the authors of libgit2.

- The libgit2 printf calls in cache.c and util.c have been modified to
  use the R printing routine Rprintf.

License
-------

The `git2r` package is licensed under the GPLv2. See these files for additional details:

- LICENSE      - `git2r` package license (GPLv2)
- inst/COPYING - Copyright notices for additional included software


---

[![](http://ropensci.org/public_images/github_footer.png)](http://ropensci.org)
