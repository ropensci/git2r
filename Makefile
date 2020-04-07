# Determine package name and version from DESCRIPTION file
PKG_VERSION=$(shell grep -i ^version DESCRIPTION | cut -d : -d \  -f 2)
PKG_NAME=$(shell grep -i ^package DESCRIPTION | cut -d : -d \  -f 2)

# Name of built package
PKG_TAR=$(PKG_NAME)_$(PKG_VERSION).tar.gz

# Install package
install:
	cd .. && R CMD INSTALL $(PKG_NAME)

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
	cd .. && _R_CHECK_CRAN_INCOMING_=FALSE NOT_CRAN=true \
        R CMD check --as-cran --no-manual --no-vignettes \
        --no-build-vignettes --no-stop-on-test-error $(PKG_TAR)

# Build and check package on R-hub
rhub: clean check
	cd .. && Rscript -e "rhub::check(path='$(PKG_TAR)', rhub::platforms()[['name']], show_status = FALSE)"

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

# Sync git2r with changes in the libgit2 C-library
#
# 1) clone or pull libgit2 to parent directory from
# https://github.com/libgit/libgit.git
#
# 2) run 'make sync_libgit2'. It first removes files and then copy
# files from libgit2 directory. Next it runs an R script to build
# Makevars.in and Makevars.win based on source files. Finally it runs
# a patch command to change some lines in the source code to pass
# 'R CMD check git2r'
#
# 3) Build and check updated package 'make check'
sync_libgit2:
	-rm -f src/libgit2/deps/http-parser/*
	-rm -rf src/libgit2/include
	-rm -rf src/libgit2/src
	-cp -f ../libgit2/deps/http-parser/* src/libgit2/deps/http-parser
	-cp -r ../libgit2/include/ src/libgit2/include
	-rm -f src/libgit2/include/git2/inttypes.h
	-rm -f src/libgit2/include/git2/stdint.h
	-cp -r ../libgit2/src/ src/libgit2/src
	-rm -f src/libgit2/deps/http-parser/CMakeLists.txt
	-rm -f src/libgit2/deps/regex/CMakeLists.txt
	-rm -f src/libgit2/src/CMakeLists.txt
	-rm -f src/libgit2/src/features.h.in
	-rm -f src/libgit2/src/stransport_stream.c
	-rm -f src/libgit2/src/hash/hash_common_crypto.h
	-rm -f src/libgit2/src/hash/hash_generic.c
	-rm -f src/libgit2/src/hash/hash_generic.h
	-rm -f src/libgit2/src/hash/hash_mbedtls.c
	-rm -f src/libgit2/src/hash/hash_mbedtls.h
	-rm -f src/libgit2/src/hash/hash_win32.c
	-rm -f src/libgit2/src/hash/hash_win32.h
	-rm -f src/libgit2/src/transports/auth_negotiate.c
	-rm -rf src/libgit2/src/win32
	cd src/libgit2/deps/regex && patch -i ../../../../patches/regcomp-pass-R-CMD-check-git2r.patch
	cd src/libgit2/deps/regex && patch -i ../../../../patches/regex-prefix-entry-points.patch
	Rscript scripts/build_Makevars.R

Makevars:
	Rscript scripts/build_Makevars.R

configure: configure.ac
	autoconf ./configure.ac > ./configure
	chmod +x ./configure

clean:
	./cleanup
	-rm -rf ../revdep

.PHONY: all readme install roxygen sync_libgit2 Makevars check check_gctorture \
        check_valgrind revdep revdep_install revdep_check revdep_results valgrind \
        clean
