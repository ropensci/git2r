all: readme
readme: $(patsubst %.Rmd, %.md, $(wildcard *.Rmd))

%md: %Rmd
	Rscript -e "library(knitr); knit('README.Rmd', quiet = TRUE)"
	sed   's/```r/```coffee/' README.md > README2.md
	rm README.md
	mv README2.md README.md

doc:
	rm -f man/*.Rd
	cd .. && Rscript -e "library(methods); library(utils); library(roxygen2); roxygenize('git2r')"

# Sync git2r with changes in the libgit2 C-library
#
# 1) clone or pull libgit2 to parent directory from
# https://github.com/libgit/libgit.git
#
# 2) run target sync_libgit2. It first removes files and then copy files
# from libgit2 directory. Finally, it runs an R script to build
# Makevars.in and Makevars.win based on source files
#
# 3) Check cache.c and util.c. They have been modified to use R
# printing routine Rprintf. If there are no other changes in these two
# files they must be reverted to previous state to pass R CMD check
#
# 4) Build and check updated package
#    - R CMD build git2r
#    - R CMD check git2r_version.tar.gz
sync_libgit2:
	-rm -f src/http-parser/*
	-rm -f src/regex/*
	-rm -f src/libgit2/include/*.h
	-rm -f src/libgit2/include/git2/*.h
	-rm -f src/libgit2/include/git2/sys/*.h
	-rm -f src/libgit2/*.c
	-rm -f src/libgit2/*.h
	-rm -f src/libgit2/hash/*.c
	-rm -f src/libgit2/hash/*.h
	-rm -f src/libgit2/transports/*.c
	-rm -f src/libgit2/transports/*.h
	-rm -f src/libgit2/unix/*.c
	-rm -f src/libgit2/unix/*.h
	-rm -f src/libgit2/win32/*.c
	-rm -f src/libgit2/win32/*.h
	-rm -f src/libgit2/xdiff/*.c
	-rm -f src/libgit2/xdiff/*.h
	-rm -f inst/AUTHORS_libgit2
	-rm -f inst/NOTICE
	-cp -f ../libgit2/deps/http-parser/* src/http-parser
	-cp -f ../libgit2/deps/regex/* src/regex
	-cp -f ../libgit2/include/*.h src/libgit2/include
	-cp -f ../libgit2/include/git2/*.h src/libgit2/include/git2
	-cp -f ../libgit2/include/git2/sys/*.h src/libgit2/include/git2/sys
	-cp -f ../libgit2/src/*.c src/libgit2
	-cp -f ../libgit2/src/*.h src/libgit2
	-cp -f ../libgit2/src/hash/*.c src/libgit2/hash
	-cp -f ../libgit2/src/hash/*.h src/libgit2/hash
	-cp -f ../libgit2/src/transports/*.c src/libgit2/transports
	-cp -f ../libgit2/src/transports/*.h src/libgit2/transports
	-cp -f ../libgit2/src/unix/*.c src/libgit2/unix
	-cp -f ../libgit2/src/unix/*.h src/libgit2/unix
	-cp -f ../libgit2/src/win32/*.c src/libgit2/win32
	-cp -f ../libgit2/src/win32/*.h src/libgit2/win32
	-cp -f ../libgit2/src/xdiff/*.c src/libgit2/xdiff
	-cp -f ../libgit2/src/xdiff/*.h src/libgit2/xdiff
	-cp -f ../libgit2/AUTHORS inst/AUTHORS_libgit2
	-cp -f ../libgit2/COPYING inst/NOTICE
	Rscript tools/build_Makevars.r

Makevars:
	Rscript tools/build_Makevars.r

clean:
	-rm -f config.log
	-rm -f config.status
	-rm -f src/Makevars
	-rm -f src/*.o
	-rm -f src/*.so
	-rm -f src/libgit2/*.o
	-rm -f src/libgit2/hash/*.o
	-rm -f src/libgit2/transports/*.o
	-rm -f src/libgit2/unix/*.o
	-rm -f src/libgit2/win32/*.o
	-rm -f src/libgit2/xdiff/*.o
	-rm -f src/http-parser/*.o

.PHONY: all readme doc sync_libgit2 Makevars clean
