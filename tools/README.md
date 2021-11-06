# Scripts for configuring/building git2r

http://cran.r-project.org/doc/manuals/r-release/R-exts.html#Configure-and-cleanup

> If your configure script needs auxiliary files, it is recommended
> that you ship them in a tools directory (as R itself does).

The macro `AC_CANONICAL_HOST` in `configure.ac` determines the `host`
with the helper scripts `config.guess` and `config.sub`. This also
requires the script `install-sh`, a wrapper around the system's own
install utility.

## From

```
wget -O config.guess 'https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD'
wget -O config.sub 'https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD'
wget -O config.rpath 'https://git.savannah.gnu.org/gitweb/?p=gnulib.git;a=blob_plain;f=build-aux/config.rpath;hb=HEAD'
```
