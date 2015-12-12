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

## For debugging
sessionInfo()

## Check validity of S4 class git_signature
## Each slot must have length equal to one
when <- new("git_time", time = 1395567947, offset = 60)

tools::assertError(validObject(new("git_signature",
                                   when = when)))

tools::assertError(validObject(new("git_signature",
                                   name = character(0),
                                   email = "email",
                                   when = when)))

tools::assertError(validObject(new("git_signature",
                                   name = c("name1", "name2"),
                                   email = "email",
                                   when = when)))

tools::assertError(validObject(new("git_signature",
                                   name = "name",
                                   email = character(0),
                                   when = when)))

tools::assertError(validObject(new("git_signature",
                                   name = "name",
                                   email = c("email1", "email2"),
                                   when = when)))
