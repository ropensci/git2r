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

I'm developing the package on Linux, so it's very possible that other platforms currently fails to install the package. But feel free to look into that.

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
#> Local:    testing_check /Users/karthik/Documents/work/Github/ropensci/git2r/
```

```coffee

# Summary of repository
summary(repo)
```

```
#> Remote:   @ origin (https://github.com/ropensci/git2r)
#> Local:    testing_check /Users/karthik/Documents/work/Github/ropensci/git2r/
#> 
#> Branches:      8
#> Tags:          0
#> Commits:       144
#> Contributors:  2
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
#> $`refs/heads/master`
#> [772fd8] master
#> 
#> $`refs/heads/testing_check`
#> [b31eca] testing_check
#> 
#> $`refs/remotes/origin/example_fix`
#> [1dda04] origin/example_fix
#> 
#> $`refs/remotes/origin/HEAD`
#> refs/remotes/origin/HEAD => refs/remotes/origin/master
#> 
#> $`refs/remotes/origin/master`
#> [772fd8] origin/master
#> 
#> $`refs/remotes/origin/README_fix`
#> [384d3a] origin/README_fix
#> 
#> $`refs/remotes/origin/testing_check`
#> [b31eca] origin/testing_check
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
#> [772fd8] (Local) master
#> 
#> [[5]]
#> [b31eca] (Local) (HEAD) testing_check
#> 
#> [[6]]
#> [1dda04] (origin @ https://github.com/ropensci/git2r) example_fix
#> 
#> [[7]]
#> (origin @ https://github.com/ropensci/git2r) refs/remotes/origin/HEAD => refs/remotes/origin/master
#> 
#> [[8]]
#> [772fd8] (origin @ https://github.com/ropensci/git2r) master
#> 
#> [[9]]
#> [384d3a] (origin @ https://github.com/ropensci/git2r) README_fix
#> 
#> [[10]]
#> [b31eca] (origin @ https://github.com/ropensci/git2r) testing_check
#> 
#> [[11]]
#> [9089f6] (origin @ https://github.com/ropensci/git2r) travis_fix
```

```coffee

# List all commits in repository
commits(repo)[1] # Truncated here for readability
```

```
#> [[1]]
#> Commit:  b31eca2847a0f8fb7e51445f835335b69dd2cd10
#> Author:  Karthik Ram <karthik.ram@gmail.com>
#> When:    2014-03-19 17:47:59
#> Summary: Updated README with new examples.
```

```coffee

# Get HEAD of repository
head(repo)
```

```
#> [b31eca] (Local) (HEAD) testing_check
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
library(ggplot2)
library(reshape2)
contributions()
```

<img src="figure/contributionnum.png" title="plot of chunk contributionnum" alt="plot of chunk contributionnum" width="10" />




### Visualize contributions by user on a monthly timeline (another way of looking at the same data from above)


```coffee
library(git2r)
contribution_by_user()
```

<img src="figure/contributions_by_user1.png" title="plot of chunk contributions_by_user" alt="plot of chunk contributions_by_user" width="10" />

```coffee
contribution_by_user(breaks = "months", data_only = TRUE)
```

```
#>             name      month counts
#> 1    Karthik Ram 2013-12-01     NA
#> 2 Stefan Widgren 2013-12-01     35
#> 3    Karthik Ram 2014-01-01     NA
#> 4 Stefan Widgren 2014-01-01     33
#> 5    Karthik Ram 2014-02-01     NA
#> 6 Stefan Widgren 2014-02-01      7
#> 7    Karthik Ram 2014-03-01     25
#> 8 Stefan Widgren 2014-03-01     44
```

```coffee
contribution_by_user(breaks = "weeks")
```

<img src="figure/contributions_by_user2.png" title="plot of chunk contributions_by_user" alt="plot of chunk contributions_by_user" width="10" />

```coffee
contribution_by_user(breaks = "days")
```

<img src="figure/contributions_by_user3.png" title="plot of chunk contributions_by_user" alt="plot of chunk contributions_by_user" width="10" />

```coffee
 
```


### Generate a wordcloud from the commit messages in a repository


```coffee
library(git2r)
contribution_wc()
```

<img src="figure/wordcloud.png" title="plot of chunk wordcloud" alt="plot of chunk wordcloud" width="10" />



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
