git2r
=====

R bindings to [libgit2](https://github.com/libgit2/libgit2) library. The package uses the source code and headers of `libgit2` in src/libgit2 to compile and build as a R package. The libgit2 printf calls in cache.c and util.c have been modified to use the R printing routine Rprintf.

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

```
# install.packages("devtools")
library(devtools)
install_github("git2r", "stewid")
```

Example
-------

```
library(git2r)

# Open an existing repository
repo <- repository("path/to/git2r")

# Brief summary of repository
repo

# Summary of repository
summary(repo)

# Workdir of repository
workdir(repo)

# Check if repository is bare
is.bare(repo)

# Check if repository is empty
is.empty(repo)

# List all references in repository
references(repo)

# List all branches in repository
branches(repo)

# List all commits in repository
commits(repo)

# Get HEAD of repository
head(repo)

# Check if HEAD is head
is.head(head(repo))

# Check if HEAD is local
is.local(head(repo))

# List all tags in repository
tags(repo)
```

### Visualize the number of commits per month in a repository

```
library(git2r)
library(ggplot2)
library(plyr)

## Open a repository with some history
repo <- repository('path/to/libgit2')

## Harvest neccessary data from repository
df <- do.call('rbind', lapply(commits(repo), function(x) {
    data.frame(name=x@author@name,
               when=as(x@author@when, 'POSIXct'))
}))

## Format data
df$month <- as.POSIXct(cut(df$when, breaks='month'))
df <- ddply(df, ~month, nrow)
names(df) <- c('month', 'n')

## Plot data
ggplot(df, aes(x=month, y=n)) +
    geom_bar(stat='identity') +
    scale_x_datetime('Month') +
    scale_y_continuous('Count') +
    labs(title='Commits')
```

### Visualize the number of contributors per month in a repository

```
library(git2r)
library(ggplot2)
library(plyr)

## Open a repository with some history
repo <- repository('path/to/libgit2')

## Harvest neccessary data from repository
df <- do.call('rbind', lapply(commits(repo), function(x) {
    data.frame(name=x@author@name,
               when=as(x@author@when, 'POSIXct'))
}))

## Format data
df$month <- as.POSIXct(cut(df$when, breaks='month'))
df <- ddply(df, ~month, function(x) length(unique(x$name)))
names(df) <- c('month', 'n')

## Plot data
ggplot(df, aes(x=month, y=n)) +
    geom_bar(stat='identity') +
    scale_x_datetime('Month') +
    scale_y_continuous('Count') +
    labs(title='Contributors')
```

### Generate a wordcloud from the commit messages in a repository

```
library(git2r)
library(wordcloud)

## Open the libgit2 repository
repo <- repository('path/to/libgit2')

## Harvest neccessary data from repository
msg <- paste(sapply(commits(repo), slot, 'message'), collapse=' ')

## Create the wordcloud
wordcloud(msg, scale=c(5,0.5), max.words=100, random.order=FALSE,
          rot.per=0.35, use.r.layout=FALSE, colors=brewer.pal(8, 'Dark2'))
```

![wordcloud](wordcloud.png)

License
-------

The `git2r` package is licensed under the GPLv2. See these files for additional details:

- LICENSE      - `git2r` package license (GPLv2)
- inst/COPYING - Copyright notices for additional included software
