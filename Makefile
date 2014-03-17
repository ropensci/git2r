all: readme  
readme: $(patsubst %.Rmd, %.md, $(wildcard *.Rmd))
 
%md: %Rmd
	Rscript -e "library(knitr); knit('README.Rmd')";

