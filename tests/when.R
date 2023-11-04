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

library("git2r")

## For debugging
sessionInfo()
libgit2_version()
libgit2_features()


## Check when method
w1 <- structure(list(time = 1395567947, offset = 60),
               class = "git_time")
stopifnot(identical(when(w1), "2014-03-23 09:45:47 GMT"))
stopifnot(identical(when(w1, usetz = FALSE), "2014-03-23 09:45:47"))
stopifnot(identical(when(w1, tz = "Europe/Stockholm", origin = "1980-02-02"),
                    "2024-04-23 11:45:47 CEST"))

s1 <- structure(list(name = "Alice", email = "alice@example.org", when = w1),
                class = "git_signature")
stopifnot(identical(when(s1), "2014-03-23 09:45:47 GMT"))
stopifnot(identical(when(s1, usetz = FALSE), "2014-03-23 09:45:47"))
stopifnot(identical(when(s1, tz = "Europe/Stockholm", origin = "1980-02-02"),
                    "2024-04-23 11:45:47 CEST"))

w2 <- structure(list(time = 1395567950, offset = 60),
               class = "git_time")
s2 <- structure(list(name = "Alice", email = "alice@example.org", when = w2),
                class = "git_signature")
c1 <- structure(list(sha = "166f3f779fd7e4165aaa43f2828050ce040052b0",
                     author = s1,
                     committer = s2,
                     summary = "A commit summary",
                     message = "A commit message"),
                class = "git_commit")
stopifnot(identical(when(c1), "2014-03-23 09:45:47 GMT"))
stopifnot(identical(when(c1, usetz = FALSE), "2014-03-23 09:45:47"))
stopifnot(identical(when(c1, tz = "Europe/Stockholm", origin = "1980-02-02"),
                    "2024-04-23 11:45:47 CEST"))

t1 <- structure(list(sha = "166f3f779fd7e4165aaa43f2828050ce040052b0",
                     message = "A tag message",
                     name = "A tage name",
                     tagger = s1,
                     target = "166f3f779fd7e4165aaa43f2828050ce040052b0"),
                class = "git_tag")
stopifnot(identical(when(t1), "2014-03-23 09:45:47 GMT"))
stopifnot(identical(when(t1, usetz = FALSE), "2014-03-23 09:45:47"))
stopifnot(identical(when(t1, tz = "Europe/Stockholm", origin = "1980-02-02"),
                    "2024-04-23 11:45:47 CEST"))
