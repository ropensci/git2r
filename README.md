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
```

```
#> Loading required package: methods
```

```coffee

# Open an existing repository
# repo <- repository("path/to/git2r")
repo <- repository(getwd())

# Brief summary of repository
repo
```

```
#> Remote:   @ origin (https://github.com/ropensci/git2r.git)
#> Local:    master /home/stefan/projects/packages/git2r/git2r/
```

```coffee

# Summary of repository
summary(repo)
```

```
#> Remote:   @ origin (https://github.com/ropensci/git2r.git)
#> Local:    master /home/stefan/projects/packages/git2r/git2r/
#> 
#> Branches:      4
#> Tags:          0
#> Commits:       175
#> Contributors:  3
```

```coffee

# Workdir of repository
workdir(repo)
```

```
#> [1] "/home/stefan/projects/packages/git2r/git2r/"
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
#> $`refs/heads/data.frame`
#> [772fd8] data.frame
#> 
#> $`refs/heads/master`
#> [27943f] master
#> 
#> $`refs/heads/tag`
#> [620067] tag
#> 
#> $`refs/remotes/origin/HEAD`
#> refs/remotes/origin/HEAD => refs/remotes/origin/master
#> 
#> $`refs/remotes/origin/README_fix`
#> [384d3a] origin/README_fix
#> 
#> $`refs/remotes/origin/master`
#> [27943f] origin/master
#> 
#> $`refs/stash`
#> [4e2015] stash
```

```coffee

# List all branches in repository
branches(repo)
```

```
#> [[1]]
#> [772fd8] (Local) data.frame
#> 
#> [[2]]
#> [27943f] (Local) (HEAD) master
#> 
#> [[3]]
#> [620067] (Local) tag
#> 
#> [[4]]
#> (origin @ https://github.com/ropensci/git2r.git) refs/remotes/origin/HEAD => refs/remotes/origin/master
#> 
#> [[5]]
#> [384d3a] (origin @ https://github.com/ropensci/git2r.git) README_fix
#> 
#> [[6]]
#> [27943f] (origin @ https://github.com/ropensci/git2r.git) master
```

```coffee

# List all commits in repository
commits(repo)[1] # Truncated here for readability
```

```
#> [[1]]
#> Commit:  27943f7f9eebe955730594da3da78b6d52457799
#> Author:  Stefan Widgren <stefan.widgren@gmail.com>
#> When:    2014-03-23 10:07:09
#> Summary: Fixed plot by user
```

```coffee

# Get HEAD of repository
head(repo)
```

```
#> [27943f] (Local) (HEAD) master
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

# Open an existing repository
# repo <- repository("path/to/git2r")
repo <- repository(getwd())

contributions(repo)
```

```
#>         when   n
#> 1 2013-12-01  35
#> 2 2014-01-01  33
#> 3 2014-02-01   7
#> 4 2014-03-01 100
```

```coffee

plot(repo)
```

![plot of chunk contributionnum](figure/contributionnum.png) 


### Visualize contributions by user on a monthly timeline (another way of looking at the same data from above)


```coffee
library(git2r)

# Open an existing repository
# repo <- repository("path/to/git2r")
repo <- repository(getwd())

contributions(repo, by='user')
```

```
#>         when            author  n
#> 1 2013-12-01    Stefan Widgren 35
#> 2 2014-01-01    Stefan Widgren 33
#> 3 2014-02-01    Stefan Widgren  7
#> 4 2014-03-01       Karthik Ram 31
#> 5 2014-03-01 Scott Chamberlain  2
#> 6 2014-03-01    Stefan Widgren 67
```

```coffee

plot(repo, by = "user")
```

![plot of chunk contributions_by_user](figure/contributions_by_user1.png) 

```coffee
plot(repo, breaks="week", by = "user")
```

![plot of chunk contributions_by_user](figure/contributions_by_user2.png) 

```coffee
plot(repo, breaks="day", by = "user")
```

![plot of chunk contributions_by_user](figure/contributions_by_user3.png) 


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
