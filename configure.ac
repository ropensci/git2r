# git2r, R bindings to the libgit2 library.
# Copyright (C) 2013-2014 The git2r contributors
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
have_zlib=no
AC_SEARCH_LIBS([inflate], [z], [have_zlib=yes])

if test "x${have_zlib}" = xno; then
        AC_MSG_FAILURE([zlib library required])
fi

# Check for OpenSSL
have_ssl=no
OLD_LIBS=${LIBS}
AC_SEARCH_LIBS([SSL_library_init], [ssl], [have_ssl=yes])

have_crypto=no
if test "x${have_ssl}" = xyes; then
        AC_SEARCH_LIBS([EVP_EncryptInit], [crypto], [have_crypto=yes])
fi

if test "x${have_crypto}" = xyes; then
        GIT2R_HAVE_SSL=-DGIT_SSL
else
        LIBS=${OLD_LIBS}
        AC_MSG_WARN([
  -----------------------------------------------
   Unable to find OpenSSL on this system.
   Building a version without support for HTTPS.
  -----------------------------------------------])
fi

# Check for LibSSH2
have_ssh2=no
AC_SEARCH_LIBS([libssh2_version], [ssh2], [have_ssh2=yes])

if test "x${have_ssh2}" = xyes; then
        GIT2R_HAVE_SSH2=-DGIT_SSH
else
        AC_MSG_WARN([
  ---------------------------------------------
   Unable to find libSSH2 on this system.
   Building a version without support for SSH.
  ---------------------------------------------])
fi

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

AC_SUBST(GIT2R_HAVE_SSL)
AC_SUBST(GIT2R_HAVE_SSH2)
AC_SUBST(CPPFLAGS)
AC_SUBST(LIBS)

AC_CONFIG_FILES([src/Makevars])

AC_OUTPUT
