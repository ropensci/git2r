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
git clone http://git.savannah.gnu.org/cgit/automake.git
git checkout 4cf99902de8708538843d9fc173549fde4868159
cp automake/lib/config.guess /path/to/git2r/tools/
cp automake/lib/config.sub /path/to/git2r/tools/
cp automake/lib/install-sh /path/to/git2r/tools/
cp automake/lib/missing /path/to/git2r/tools/
```
