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

##' Bundle bare repo of package
##'
##' Clone the package git repository as a bare repository to
##' \code{pkg/inst/pkg.git}
##' @template repo-param
##' @return Invisible bundled \code{git_repository} object
##' @export
##' @examples
##' \dontrun{
##' ## Initialize repository
##' path <- tempfile()
##' dir.create(path)
##' path <- file.path(path, "git2r")
##' repo <- clone("https://github.com/ropensci/git2r.git", path)
##'
##' ## Bundle bare repository in package
##' bundle_r_package(repo)
##'
##' ## Build and install bundled package
##' wd <- setwd(dirname(path))
##' system(sprintf("R CMD build %s", path))
##' pkg <- list.files(".", pattern = "[.]tar[.]gz$")
##' system(sprintf("R CMD INSTALL %s", pkg))
##' setwd(wd)
##'
##' ## Reload package
##' detach("package:git2r", unload = TRUE)
##' library(git2r)
##'
##' ## Summarize last five commits of bundled repo
##' repo <- repository(system.file("git2r.git", package = "git2r"))
##' invisible(lapply(commits(repo, n = 5), summary))
##'
##' ## Plot content of bundled repo
##' plot(repo)
##' }
bundle_r_package <- function(repo = ".") {
    repo <- lookup_repository(repo)

    ## Check for 'inst' folder
    inst <- file.path(workdir(repo), "inst")
    if (!isTRUE(file.info(inst)$isdir))
        dir.create(inst)

    ## Check for 'pkg.git' folder
    local_path <- paste0(basename(workdir(repo)), ".git", sep = "")
    local_path <- file.path(inst, local_path)
    if (file.exists(local_path))
        stop("Repo already exists:", local_path)
    invisible(clone(workdir(repo), local_path, bare = TRUE))
}
