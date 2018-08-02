## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2018 The git2r contributors
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

## Test to coerce
git_t <- structure(list(time = 1395567947, offset = 60),
                   class = "git_time")
stopifnot(identical(as.character(git_t), "2014-03-23 10:45:47"))
stopifnot(identical(as.POSIXct(git_t),
                    as.POSIXct(1395571547, origin="1970-01-01", tz="GMT")))
stopifnot(identical(print(git_t), git_t))
