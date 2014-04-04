## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2014  Stefan Widgren
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, version 2 of the License.
##
## git2r is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.

library(git2r)

##
## Check validity of S4 class git_signature
## Each slot must have length equal to one
##
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
