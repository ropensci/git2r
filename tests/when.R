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
## Check when method
##
stopifnot(identical(when(new("git_commit",
                             hex = "166f3f779fd7e4165aaa43f2828050ce040052b0",
                             author = new("git_signature",
                                 name = "Stefan Widgren",
                                 email = "stefan.widgren@gmail.com",
                                 when = new("git_time", time = 1395567947, offset = 60)),
                             committer = new("git_signature",
                                 name = "Stefan Widgren",
                                 email = "stefan.widgren@gmail.com",
                                 when = new("git_time", time = 1395567950, offset = 60)),
                             summary = "A commit summary",
                             message = "A commit message")),
                    "2014-03-23 10:45:47"))

stopifnot(identical(when(new("git_signature",
                             name = "Stefan Widgren",
                             email = "stefan.widgren@gmail.com",
                             when = new("git_time", time = 1395567947, offset = 60))),
                    "2014-03-23 10:45:47"))

stopifnot(identical(when(new("git_tag",
                             message = "A tag message",
                             name = "A tage name",
                             tagger = new("git_signature",
                                 name = "Stefan Widgren",
                                 email = "stefan.widgren@gmail.com",
                                 when = new("git_time", time = 1395567947, offset = 60)),
                             target = "166f3f779fd7e4165aaa43f2828050ce040052b0")),
                    "2014-03-23 10:45:47"))

stopifnot(identical(when(new("git_time", time = 1395567947, offset = 60)),
                    "2014-03-23 10:45:47"))
