git2r
=====

R bindings to the libgit2 library

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

    # install.packages("devtools")
    library(devtools)
    install_github("git2r", "stewid")

Example
-------

    library(git2r)

    # Open an existing repository
    repo <- repository('path/to/git2r')

    # Check if repository is bare
    is.bare(repo)

    # Check if repository is empty
    is.empty(repo)
