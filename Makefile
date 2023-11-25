# Determine package name and version from DESCRIPTION file
PKG_VERSION=$(shell grep -i ^version DESCRIPTION | cut -d : -d \  -f 2)
PKG_NAME=$(shell grep -i ^package DESCRIPTION | cut -d : -d \  -f 2)

# Name of built package
PKG_TAR=$(PKG_NAME)_$(PKG_VERSION).tar.gz

# Install package
install:
	cd .. && R CMD INSTALL $(PKG_NAME)

# Check visibility of C entry points
check_visibility:
	cd .. && R CMD INSTALL $(PKG_NAME)
	nm -g src/git2r.so | grep " T " | grep 2 && exit 0 || exit 1

# Build documentation with roxygen
# 1) Remove old doc
# 2) Generate documentation
roxygen:
	rm -f man/*.Rd
	cd .. && Rscript -e "roxygen2::roxygenize('$(PKG_NAME)')"

# Generate PDF output from the Rd sources
# 1) Rebuild documentation with roxygen
# 2) Generate pdf, overwrites output file if it exists
pdf: roxygen
	cd .. && R CMD Rd2pdf --force $(PKG_NAME)

# Build and check package
check:
	cd .. && R CMD build --no-build-vignettes $(PKG_NAME)
	cd .. && _R_CHECK_CRAN_INCOMING_=FALSE _R_CHECK_SYSTEM_CLOCK_=0 \
        R CMD check --as-cran --no-manual --no-vignettes \
        --no-build-vignettes --no-stop-on-test-error $(PKG_TAR)

# Build and check package on R-hub
rhub: clean check
	cd .. && Rscript -e "rhub::check(path='$(PKG_TAR)', rhub::platforms()[['name']], show_status = FALSE)"

# Build and check package on https://win-builder.r-project.org/
.PHONY: winbuilder
winbuilder: clean check
	cd .. && curl -T $(PKG_TAR) ftp://win-builder.r-project.org/R-oldrelease/
	cd .. && curl -T $(PKG_TAR) ftp://win-builder.r-project.org/R-release/
	cd .. && curl -T $(PKG_TAR) ftp://win-builder.r-project.org/R-devel/

# Check reverse dependencies
#
# 1) Install packages (in ../revdep/lib) to check the reverse dependencies.
# 2) Check the reverse dependencies using 'R CMD check'.
# 3) Collect results from the '00check.log' files.
revdep: revdep_install revdep_check revdep_results

# Install packages to check reverse dependencies
revdep_install: clean
	mkdir -p ../revdep/lib
	cd .. && R CMD INSTALL --library=revdep/lib $(PKG_NAME)
	R_LIBS_USER=../revdep/lib Rscript --vanilla \
          -e "options(repos = c(CRAN='https://cran.r-project.org'))" \
          -e "pkg <- tools::package_dependencies('$(PKG_NAME)', which = 'all', reverse = TRUE)" \
          -e "pkg <- as.character(unlist(pkg))" \
          -e "dep <- sapply(pkg, tools::package_dependencies, which = 'all')" \
          -e "dep <- as.character(unlist(dep))" \
          -e "if ('BiocInstaller' %in% dep) {" \
          -e "    source('https://bioconductor.org/biocLite.R')" \
          -e "    biocLite('BiocInstaller')" \
          -e "}" \
          -e "install.packages(pkg, dependencies = TRUE)" \
          -e "download.packages(pkg, destdir = '../revdep')"

# Check reverse dependencies with 'R CMD check'
revdep_check:
	$(foreach var,$(wildcard ../revdep/*.tar.gz),R_LIBS_USER=../revdep/lib \
          _R_CHECK_CRAN_INCOMING_=FALSE R --vanilla CMD check --as-cran \
          --no-stop-on-test-error --output=../revdep $(var) \
          | tee --append ../revdep/00revdep.log;)

# Collect results from checking reverse dependencies
revdep_results:
	Rscript --vanilla \
          -e "options(repos = c(CRAN='https://cran.r-project.org'))" \
          -e "pkg <- tools::package_dependencies('$(PKG_NAME)', which = 'all', reverse = TRUE)" \
          -e "pkg <- as.character(unlist(pkg))" \
          -e "results <- do.call('rbind', lapply(pkg, function(x) {" \
          -e "    filename <- paste0('../revdep/', x, '.Rcheck/00check.log')" \
          -e "    if (file.exists(filename)) {" \
          -e "        lines <- readLines(filename)" \
          -e "        status <- sub('^Status: ', '', lines[grep('^Status: ', lines)])" \
          -e "    } else {" \
          -e "        status <- 'missing'" \
          -e "    }" \
          -e "    data.frame(Package = x, Status = status)" \
          -e "}))" \
          -e "results <- results[order(results[, 'Status']), ]" \
          -e "rownames(results) <- NULL" \
          -e "cat('\n\n*** Results ***\n\n')" \
          -e "results" \
          -e "cat('\n\n')"

# Build and check package with gctorture
check_gctorture:
	cd .. && R CMD build --no-build-vignettes $(PKG_NAME)
	cd .. && R CMD check --no-manual --no-vignettes --no-build-vignettes --use-gct $(PKG_TAR)

# Build and check package with valgrind
check_valgrind:
	cd .. && R CMD build --no-build-vignettes $(PKG_NAME)
	cd .. && R CMD check --as-cran --no-manual --no-vignettes --no-build-vignettes --use-valgrind $(PKG_TAR)

# Run all tests with valgrind
test_objects = $(wildcard tests/*.R)
valgrind:
	$(foreach var,$(test_objects),R -d "valgrind --tool=memcheck --leak-check=full" --vanilla < $(var);)

configure: configure.ac
	autoconf ./configure.ac > ./configure
	chmod +x ./configure

clean:
	./cleanup
	-rm -rf ../revdep

.PHONY: all readme install roxygen sync_libgit2 Makevars check check_gctorture \
        check_valgrind revdep revdep_install revdep_check revdep_results valgrind \
        clean
