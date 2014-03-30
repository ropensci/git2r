# git2r, R bindings to the libgit2 library.
# Copyright (C) 2013-2014  Stefan Widgren
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 2 of the License.
#
# git2r is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

AC_PREREQ([2.69])
AC_INIT([git2r], [see.DESCRIPTION.file], [stefan.widgren@gmail.com])
AC_CONFIG_SRCDIR([src/git2r.c])

# Checks for programs.
AC_PROG_CC

# zlib include and library path
AC_ARG_WITH([zlib-include],
    AC_HELP_STRING([--with-zlib-include=INCLUDE_PATH],
                   [the location of zlib header files]),
    [zlib_include_path=$withval])
if test [ -n "$zlib_include_path" ] ; then
  GIT2R_CPPFLAGS="-I. -I${zlib_include_path}"
else
  GIT2R_CPPFLAGS="-I."
fi

AC_ARG_WITH([zlib-lib],
    AC_HELP_STRING([--with-zlib-lib=LIB_PATH],
                   [the location of zlib libraries]),
    [zlib_lib_path=$withval])
if test [ -n "$zlib_lib_path" ] ; then
  LIBS="-L${zlib_lib_path} ${LIBS}"
fi

# Find the compiler and compiler flags to use
: ${R_HOME=`R RHOME`}
if test -z "${R_HOME}"; then
  echo "could not determine R_HOME"
  exit 1
fi
CC=`"${R_HOME}/bin/R" CMD config CC`
CFLAGS=`"${R_HOME}/bin/R" CMD config CFLAGS`
CPPFLAGS=`"${R_HOME}/bin/R" CMD config CPPFLAGS`

CPPFLAGS="${CPPFLAGS} ${GIT2R_CPPFLAGS}"

# Check for zlib
AC_CHECK_LIB(z, inflate,
[
        GIT2R_HAVE_ZLIB=TRUE
        AC_DEFINE_UNQUOTED([HAVE_ZLIB],[1],[Have zlib])
        AC_DEFINE_UNQUOTED([ZLIB_LIBS],[-lz],[])
        LIBS="${LIBS} -lz"
],
[
        GIT2R_HAVE_ZLIB=FALSE
])
AC_SUBST(GIT2R_HAVE_ZLIB)

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h langinfo.h libintl.h limits.h locale.h malloc.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/param.h sys/socket.h sys/time.h unistd.h wchar.h wctype.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([btowc clock_gettime getcwd gethostbyname gettimeofday isascii memchr memmove memset munmap pstat_getdynamic re_comp realpath regcomp select socket strcasecmp strchr strcspn strdup strerror strncasecmp strrchr strspn strstr strtol strtoul])

AC_SUBST(CPPFLAGS)
AC_SUBST(LIBS)

AC_CONFIG_FILES([src/Makevars])

AC_OUTPUT
