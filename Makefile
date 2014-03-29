all: readme
readme: $(patsubst %.Rmd, %.md, $(wildcard *.Rmd))

%md: %Rmd
	Rscript -e "library(knitr); knit('README.Rmd', quiet = TRUE)"
	sed   's/```r/```coffee/' README.md > README2.md
	rm README.md
	mv README2.md README.md

doc:
	rm -f man/*.Rd
	Rscript -e "library(methods); library(utils); library(devtools); document(); check_doc()"

.PHONY: all readme doc
