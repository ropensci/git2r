## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2015 The git2r contributors
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

## Check when method
stopifnot(identical(when(new("git_commit",
                             sha = "166f3f779fd7e4165aaa43f2828050ce040052b0",
                             author = new("git_signature",
                                 name = "Alice",
                                 email = "alice@example.org",
                                 when = new("git_time", time = 1395567947, offset = 60)),
                             committer = new("git_signature",
                                 name = "Alice",
                                 email = "alice@example.org",
                                 when = new("git_time", time = 1395567950, offset = 60)),
                             summary = "A commit summary",
                             message = "A commit message")),
                    "2014-03-23 10:45:47"))

stopifnot(identical(when(new("git_signature",
                             name = "Alice",
                             email = "alice@example.org",
                             when = new("git_time", time = 1395567947, offset = 60))),
                    "2014-03-23 10:45:47"))

stopifnot(identical(when(new("git_tag",
                             sha = "166f3f779fd7e4165aaa43f2828050ce040052b0",
                             message = "A tag message",
                             name = "A tage name",
                             tagger = new("git_signature",
                                 name = "Alice",
                                 email = "alice@example.org",
                                 when = new("git_time", time = 1395567947, offset = 60)),
                             target = "166f3f779fd7e4165aaa43f2828050ce040052b0")),
                    "2014-03-23 10:45:47"))

stopifnot(identical(when(new("git_time", time = 1395567947, offset = 60)),
                    "2014-03-23 10:45:47"))
