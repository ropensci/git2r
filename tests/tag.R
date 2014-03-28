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
## Check validity of S4 class git_tag
## Each slot must have length equal to one
##
when <- new('git_time', time = 1395567947, offset = 60)
tagger <- new('git_signature',
              name = 'Stefan Widgren',
              email = 'stefan.widgren@gmail.com',
              when = when)

tools::assertError(validObject(new('git_tag',
                                   message = character(0),
                                   name = 'name1',
                                   tagger = tagger,
                                   target = 'target1')))

tools::assertError(validObject(new('git_tag',
                                   message = c('message1', 'message2'),
                                   name = 'name1',
                                   tagger = tagger,
                                   target = 'target1')))

tools::assertError(validObject(new('git_tag',
                                   message = 'message1',
                                   name = character(0),
                                   tagger = tagger,
                                   target = 'target1')))

tools::assertError(validObject(new('git_tag',
                                   message = 'message1',
                                   name = c('name1', 'name2'),
                                   tagger = tagger,
                                   target = 'target1')))

tools::assertError(validObject(new('git_tag',
                                   message = 'message1',
                                   name = 'name1',
                                   tagger = tagger,
                                   target = character(0))))

tools::assertError(validObject(new('git_tag',
                                   message = 'message1',
                                   name = 'name1',
                                   tagger = tagger,
                                   target = c('target1', 'target2'))))
