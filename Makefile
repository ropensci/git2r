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
# First clone or pull libgit2 to parent directory
# from https://github.com/libgit/libgit.git
sync_libgit2:
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

.PHONY: all readme doc sync_libgit2 clean
