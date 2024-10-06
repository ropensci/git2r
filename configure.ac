# git2r, R bindings to the libgit2 library.
# Copyright (C) 2013-2024 The git2r contributors
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2,
# as published by the Free Software Foundation.
#
# git2r is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

AC_PREREQ([2.69])
AC_INIT([git2r], [see.DESCRIPTION.file], [https://github.com/ropensci/git2r/issues])
AC_CONFIG_SRCDIR([src/git2r.c])
AC_CONFIG_AUX_DIR([tools])
AC_CANONICAL_HOST

m4_include([tools/pkg.m4])

## Check for brew on macOS
AC_DEFUN([AC_PROG_BREW], [AC_CHECK_PROG(BREW,brew,yes)])

# Check for pkg-config
PKG_PROG_PKG_CONFIG

# Check for R
: ${R_HOME=`R RHOME`}
if test -z "${R_HOME}"; then
    AC_MSG_FAILURE([Could not determine R_HOME])
fi
RBIN="${R_HOME}/bin/R"

# Library settings
m4_pattern_allow([PKG_CONFIG_NAME])
m4_pattern_allow([PKG_BREW_NAME])
PKG_CONFIG_NAME="libgit2"
PKG_BREW_NAME="libgit2"
PKG_LIBS="-lgit2"
PKG_CFLAGS=""

# The minimum version of libgit2 that is compatible with git2r.
LIBGIT2_MIN_VERSION=1.0

# Check if building against bundled libgit2 or system libgit2
if test "x${INCLUDE_DIR}${LIB_DIR}" = x; then
    if test [ -n "$PKG_CONFIG" ] ; then
        # Check if libgit2 is installed and have a version that is
        # compatible with git2r.
        if $PKG_CONFIG ${PKG_CONFIG_NAME} --atleast-version=${LIBGIT2_MIN_VERSION}; then
            PKGCONFIG_CFLAGS=`"${PKG_CONFIG}" --cflags "${PKG_CONFIG_NAME}"`
            PKGCONFIG_LIBS=`"${PKG_CONFIG}" --libs "${PKG_CONFIG_NAME}"`
        fi
    fi

    if test "x${PKGCONFIG_CFLAGS}${PKGCONFIG_LIBS}" = x; then
        case "${host_os}" in
            darwin*)
                AC_PROG_BREW
                if test "x${BREW}" = xyes; then
                    BREWDIR=`brew --prefix`
                else
                    curl -sfL "https://autobrew.github.io/scripts/libgit2" > autobrew
                    . ./autobrew
                fi
            ;;
        esac
    else
        echo "Found pkg-config cflags and libs!"
        PKG_CFLAGS="${PKGCONFIG_CFLAGS}"
        PKG_LIBS="${PKGCONFIG_LIBS}"
    fi
else
    echo "Found INCLUDE_DIR and/or LIB_DIR!"
    PKG_CFLAGS="-I${INCLUDE_DIR} ${PKG_CFLAGS}"
    PKG_LIBS="-L${LIB_DIR} ${PKG_LIBS}"
fi

# Find the compiler and compiler flags to use
CC=`"${RBIN}" CMD config CC`
CFLAGS=`"${RBIN}" CMD config CFLAGS`
CPPFLAGS=`"${RBIN}" CMD config CPPFLAGS`

# check that the version of libgit2 works with git2r.
AC_MSG_CHECKING([whether the libgit2 version will work in git2r])
libgit2_ver_ok=no
${CC} ${CPPFLAGS} ${PKG_CFLAGS} ${CFLAGS} -E tools/version.c >/dev/null 2>&1 && libgit2_ver_ok=yes
AC_MSG_RESULT([${libgit2_ver_ok}])

if test "x${libgit2_ver_ok}" = xno; then
    echo "
  -----------------------------------------------------------------------

   Unable to find 'libgit2 >= ${LIBGIT2_MIN_VERSION}' on this system, please install:
     libgit2-dev   (package on e.g. Debian and Ubuntu)
     libgit2-devel (package on e.g. Fedora, CentOS and RHEL)
     libgit2       (Homebrew package on OS X)
   and try again.

   If the libgit2 library is installed on your system but the git2r
   configuration is unable to find it, you can specify the include and
   lib path to libgit2 with:

   given you downloaded a tar-gz archive:
   R CMD INSTALL git2r-.tar.gz --configure-vars='INCLUDE_DIR=/path/to/include LIB_DIR=/path/to/lib'

   or cloned the GitHub git2r repository into a directory:
   R CMD INSTALL git2r/ --configure-vars='INCLUDE_DIR=/path/to/include LIB_DIR=/path/to/lib'

   or download and install git2r in R using
   install.packages('git2r', type='source', configure.vars='LIB_DIR=-L/path/to/libs INCLUDE_DIR=-I/path/to/headers')

   On macOS, another possibility is to let the configuration
   automatically download the libgit2 library from the Homebrew
   package manager with:

   R CMD INSTALL git2r-.tar.gz --configure-vars='autobrew=yes'
   or
   R CMD INSTALL git2r/ --configure-vars='autobrew=yes'
   or
   install.packages('git2r', type='source', configure.vars='autobrew=yes')

  -----------------------------------------------------------------------

"

    AC_MSG_FAILURE([package dependency requirement 'libgit2 >= ${LIBGIT2_MIN_VERSION}' could not be satisfied.])
fi

# For debugging
echo "----- Results of the git2r package configure -----"
echo ""
echo "  PKG_CFLAGS: ${PKG_CFLAGS}"
echo "  PKG_LIBS: ${PKG_LIBS}"
echo ""
echo "--------------------------------------------------"

AC_SUBST([PKG_CFLAGS], ["${PKG_CFLAGS}"])
AC_SUBST([PKG_LIBS], ["${PKG_LIBS}"])
AC_CONFIG_FILES([src/Makevars])

AC_OUTPUT
################# End configuration to use system libgit2 ####################
