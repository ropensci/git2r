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






```r
# install.packages("devtools")
library(devtools)
install_github("git2r", "ropensci")
```


Example
-------


```r
library(git2r)
```

```
#> Loading required package: methods
```

```r

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

```r

# Summary of repository
summary(repo)
```

```
#> Remote:   @ origin (https://github.com/ropensci/git2r.git)
#> Local:    master /home/stefan/projects/packages/git2r/git2r/
#> 
#> Branches:      4
#> Tags:          0
#> Commits:       157
#> Contributors:  3
```

```r

# Workdir of repository
workdir(repo)
```

```
#> [1] "/home/stefan/projects/packages/git2r/git2r/"
```

```r

# Check if repository is bare
is.bare(repo)
```

```
#> [1] FALSE
```

```r

# Check if repository is empty
is.empty(repo)
```

```
#> [1] FALSE
```

```r

# List all references in repository
references(repo)
```

```
#> $`refs/heads/data.frame`
#> [772fd8] data.frame
#> 
#> $`refs/heads/master`
#> [a98a5f] master
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
#> [a98a5f] origin/master
#> 
#> $`refs/stash`
#> [4e2015] stash
```

```r

# List all branches in repository
branches(repo)
```

```
#> [[1]]
#> [772fd8] (Local) data.frame
#> 
#> [[2]]
#> [a98a5f] (Local) (HEAD) master
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
#> [a98a5f] (origin @ https://github.com/ropensci/git2r.git) master
```

```r

# List all commits in repository
commits(repo)[1] # Truncated here for readability
```

```
#> [[1]]
#> Commit:  a98a5fbf3748598132d57372609eb52a676ba8a7
#> Author:  Stefan Widgren <stefan.widgren@gmail.com>
#> When:    2014-03-21 22:54:19
#> Summary: Minor updates to S4 class git_repository documentation
```

```r

# Get HEAD of repository
head(repo)
```

```
#> [a98a5f] (Local) (HEAD) master
```

```r

# Check if HEAD is head
is.head(head(repo))
```

```
#> [1] TRUE
```

```r

# Check if HEAD is local
is.local(head(repo))
```

```
#> [1] TRUE
```

```r

# List all tags in repository
tags(repo)
```

```
#> named list()
```


### Visualize the number of commits per month in a repository


```r
library(git2r)
contributions()
```

![plot of chunk contributionnum](figure/contributionnum.png) 




### Visualize contributions by user on a monthly timeline (another way of looking at the same data from above)


```r
library(git2r)
contribution_by_user()
```

![plot of chunk contributions_by_user](figure/contributions_by_user1.png) 

```r
contribution_by_user(breaks = "months", data_only = TRUE)
```

```
#>                 name      month counts
#> 1     Stefan Widgren 2013-12-01     35
#> 2  Scott Chamberlain 2013-12-01     NA
#> 3        Karthik Ram 2013-12-01     NA
#> 4     Stefan Widgren 2014-01-01     33
#> 5  Scott Chamberlain 2014-01-01     NA
#> 6        Karthik Ram 2014-01-01     NA
#> 7     Stefan Widgren 2014-02-01      7
#> 8  Scott Chamberlain 2014-02-01     NA
#> 9        Karthik Ram 2014-02-01     NA
#> 10    Stefan Widgren 2014-03-01     51
#> 11 Scott Chamberlain 2014-03-01      2
#> 12       Karthik Ram 2014-03-01     29
```

```r
contribution_by_user(breaks = "weeks")
```

![plot of chunk contributions_by_user](figure/contributions_by_user2.png) 

```r
contribution_by_user(breaks = "days")
```

![plot of chunk contributions_by_user](figure/contributions_by_user3.png) 


### Generate a wordcloud from the commit messages in a repository


```r
library(git2r)
library(wordcloud)
```

```
#> Loading required package: Rcpp
#> Loading required package: RColorBrewer
```

```r
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
