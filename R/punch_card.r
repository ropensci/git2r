## git2r, R bindings to the libgit2 library.
## Copyright (C) 2013-2014 The git2r contributors
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

##' Punch card
##'
##' @rdname punch_card-methods
##' @docType methods
##' @param object The repository \code{object}.
##' @return A \code{ggplot} chart
##' @keywords methods
setGeneric("punch_card",
           signature = "object",
           function(object)
           standardGeneric("punch_card"))

##' @rdname punch_card-methods
##' @import ggplot2
##' @include S4-classes.r
##' @export
setMethod("punch_card",
          signature(object = "git_repository"),
          function (object)
          {
              ## Extract information from repository
              df <- as(object, "data.frame")
              df$when <- as.POSIXlt(df$when)
              df$hour <- df$when$hour
              df$weekday <- df$when$wday

              ## Create an index and tabulate
              df$index <- paste0(df$weekday, "-", df$hour)
              df <- as.data.frame(table(df$index), stringsAsFactors=FALSE)
              names(df) <- c("index", "Commits")

              ## Convert index to weekday and hour
              df$Weekday <- sapply(strsplit(df$index, "-"), "[", 1)
              df$Weekday <- factor(df$Weekday,
                                   levels = c(6, 5, 4, 3, 2, 1, 0),
                                   labels = c("Saturday", "Friday", "Thursday",
                                       "Wednesday", "Tuesday", "Monday", "Sunday"))
              df$Hour <- as.integer(sapply(strsplit(df$index, "-"), "[", 2))

              title <- sprintf("Punch card on repository: %s", basename(workdir(object)))

              ggplot(df, aes_string(x = "Hour", y = "Weekday", size = "Commits")) +
                  geom_point() +
                  scale_x_continuous(breaks = 0:23, limits = c(0, 23)) +
                  scale_y_discrete(labels = levels(df$Weekday)) +
                  theme(axis.title.x = element_blank(),
                        axis.title.y = element_blank()) +
                  ggtitle(title)
          }
)
