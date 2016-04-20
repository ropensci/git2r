# git2r, R bindings to the libgit2 library.
# Copyright (C) 2013-2016 The git2r contributors
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
m4_include([tools/libcurl.m4])
AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CC

## check for brew on OS X
AC_DEFUN([AC_PROG_BREW], [AC_CHECK_PROG(BREW,brew,yes)])
case "${host_os}" in
    darwin*)
        AC_PROG_BREW
        if test "x${BREW}" = xyes; then
            BREW_DIR=`brew --prefix`
        fi
    ;;
esac

# Optional include and library paths

# zlib include and library path
AC_ARG_WITH([zlib-include],
            AC_HELP_STRING([--with-zlib-include=INCLUDE_PATH],
                           [the location of the zlib header files]),
            [zlib_include_path=$withval])

AC_ARG_WITH([zlib-lib],
            AC_HELP_STRING([--with-zlib-lib=LIB_PATH],
                           [the location of the zlib library]),
            [zlib_lib_path=$withval])

# LibSSH2 include and library path
AC_ARG_WITH([libssh2-include],
            AC_HELP_STRING([--with-libssh2-include=INCLUDE_PATH],
                           [the location of the LibSSH2 header files]),
            [libssh2_include_path=$withval])

AC_ARG_WITH([libssh2-lib],
            AC_HELP_STRING([--with-libssh2-lib=LIB_PATH],
                           [the location of the LibSSH2 library]),
            [libssh2_lib_path=$withval])

# libssl include and library path
AC_ARG_WITH([libssl-include],
            AC_HELP_STRING([--with-libssl-include=INCLUDE_PATH],
                           [the location of the libssl header files]),
            [libssl_include_path=$withval])

AC_ARG_WITH([libssl-lib],
            AC_HELP_STRING([--with-libssl-lib=LIB_PATH],
                           [the location of the libssl library]),
            [libssl_lib_path=$withval])

# Find the compiler and compiler flags to use
: ${R_HOME=`R RHOME`}
if test -z "${R_HOME}"; then
  echo "could not determine R_HOME"
  exit 1
fi
RBIN="${R_HOME}/bin/R"
CC=`"${RBIN}" CMD config CC`
CFLAGS=`"${RBIN}" CMD config CFLAGS`
CPPFLAGS=`"${RBIN}" CMD config CPPFLAGS`

# Check for zlib
have_zlib=no

# Check for optional zlib include path
if test [ -n "$zlib_include_path" ] ; then
    CPPFLAGS="-I${zlib_include_path} ${CPPFLAGS}"
fi

# Check for optional zlib lib path
if test [ -n "$zlib_lib_path" ] ; then
    LIBS="-L${zlib_lib_path} ${LIBS}"
fi

AC_SEARCH_LIBS([inflate], [z], [have_zlib=yes])

if test "x${have_zlib}" = xno; then
        AC_MSG_FAILURE([
  ---------------------------------------------
   The zlib library that is required to build
   git2r was not found.

   Please install:
     zlib1g-dev (package on e.g. Debian and Ubuntu)
     zlib-devel (package on e.g. Fedora, CentOS and RHEL)
   and try again.

   If the zlib library is installed on your
   system but the git2r configuration is
   unable to find it, you can specify the
   include and lib path to zlib with:
   R CMD INSTALL --configure-args='--with-zlib-include=INCLUDE_PATH --with-zlib-lib=LIB_PATH' git2r
  ---------------------------------------------])
fi

# Check for SSL
have_ssl=no

PKG_PROG_PKG_CONFIG
AS_IF([test x != "x${PKG_CONFIG}"],
      [PKG_CHECK_MODULES([libssl], [libssl],
                         [CPPFLAGS="${libssl_CFLAGS} ${CPPFLAGS}"
                          LIBS="${libssl_LIBS} ${LIBS}"],
                         [AC_MSG_WARN("pkg-config failed to find libssl.")])],
      [AC_MSG_WARN("pkg-config not found. Proceeding without it")])

# Check for optional libssl include path
if test [ -n "$libssl_include_path" ] ; then
    CPPFLAGS="-I${libssl_include_path} ${CPPFLAGS}"
fi

# Check for optional libssl lib path
if test [ -n "$libssl_lib_path" ] ; then
    LIBS="-L${libssl_lib_path} ${LIBS}"
fi

if test "x${OPENSSL_INCLUDES}" = x; then :; else
    CPPFLAGS="${CPPFLAGS} -I${OPENSSL_INCLUDES}"
fi

# On OS X, add path to libssl in brew
case "${host_os}" in
    darwin*)
        if test "x${BREW}" = xyes; then
            CPPFLAGS="${CPPFLAGS} -I${BREW_DIR}/opt/openssl/include"
            LIBS="${LIBS} -L${BREW_DIR}/opt/openssl/lib"
        fi
    ;;
esac

AC_SEARCH_LIBS([EVP_EncryptInit],
               [crypto],
               [AC_SEARCH_LIBS([SSL_library_init],
                               [ssl],
                               [have_ssl=yes])])

if test "x${have_ssl}" = xno; then
    AC_MSG_FAILURE([
  ---------------------------------------------
   The OpenSSL library that is required to
   build git2r was not found.

   Please install:
     libssl-dev    (package on e.g. Debian and Ubuntu)
     openssl-devel (package on e.g. Fedora, CentOS and RHEL)
     openssl       (Homebrew package on OS X)
   and try again.

   If the OpenSSL library is installed on
   your system but the git2r configuration
   is unable to find it, you can specify
   the include and lib path to OpenSSL with:
   R CMD INSTALL --configure-args='--with-libssl-include=INCLUDE_PATH --with-libssl-lib=LIB_PATH' git2r
  ---------------------------------------------])
fi

# Check for LibSSH2
have_ssh2=no

# Check for optional libssh2 include path
if test [ -n "$libssh2_include_path" ] ; then
    CPPFLAGS="-I${libssh2_include_path} ${CPPFLAGS}"
fi

# Check for optional libssh2 lib path
if test [ -n "$libssh2_lib_path" ] ; then
    LIBS="-L${libssh2_lib_path} ${LIBS}"
fi

# On OS X, add path to libssh2 in brew
case "${host_os}" in
    darwin*)
        if test "x${BREW}" = xyes; then
            CPPFLAGS="${CPPFLAGS} -I${BREW_DIR}/opt/libssh2/include"
            LIBS="${LIBS} -L${BREW_DIR}/opt/libssh2/lib"
        fi
    ;;
esac

AC_SEARCH_LIBS([libssh2_version], [ssh2], [have_ssh2=yes])

if test "x${have_ssh2}" = xno; then
    AC_MSG_WARN([
  ---------------------------------------------
   Unable to find the LibSSH2 library on this
   system. Building a version without support
   for SSH transport.

   To build with SSH support, please install:
     libssh2-1-dev (package on e.g. Debian and Ubuntu)
     libssh2-devel (package on e.g. Fedora, CentOS and RHEL)
     libssh2       (Homebrew package on OS X)
   and try again.

   If the LibSSH2 library is installed on
   your system but the git2r configuration
   is unable to find it, you can specify
   the include and lib path to LibSSH2 with:
   R CMD INSTALL --configure-args='--with-libssh2-include=INCLUDE_PATH --with-libssh2-lib=LIB_PATH' git2r
  ---------------------------------------------])
fi

# Check for iconv
have_iconv=no
case "${host_os}" in
    darwin*)
        AM_ICONV
        if test "x${am_cv_func_iconv}" = xyes; then
            have_iconv=yes
        fi
    ;;
esac

# Check for libcurl
LIBCURL_CHECK_CONFIG(, , [have_libcurl=yes], [have_libcurl=no])

# Configuration specific for solaris
case "${host_os}" in
    solaris*)
        AC_SEARCH_LIBS(connect, [socket])
        AC_SEARCH_LIBS(gethostbyname, [nsl socket])

        # Include and use regex on solaris
        CPPFLAGS="-Ilibgit2/deps/regex ${CPPFLAGS}"
        GIT2R_SRC_REGEX=libgit2/deps/regex/regex.o
    ;;
esac

# Add include paths for git2r
CPPFLAGS="-I. -Ilibgit2/src -Ilibgit2/include -Ilibgit2/deps/http-parser ${CPPFLAGS}"

# Add definitions
CPPFLAGS="${CPPFLAGS} -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -DGIT_OPENSSL"

# Specify sha1 implementation
case "${host_os}" in
    darwin*)
        PKG_CFLAGS="-Wno-deprecated-declarations"
        CPPFLAGS="${CPPFLAGS} -DGIT_COMMON_CRYPTO"
    ;;
    *)
        CPPFLAGS="${CPPFLAGS} -DOPENSSL_SHA1"
    ;;
esac

# Add definition for ssh
if test "x${have_ssh2}" = xyes; then
    CPPFLAGS="${CPPFLAGS} -DGIT_SSH"
fi

# Add definition for curl
if test "x${have_libcurl}" = xyes; then
    CPPFLAGS="${CPPFLAGS} -DGIT_CURL"
fi

# Add definition for iconv
if test "x${have_iconv}" = xyes; then
    CPPFLAGS="${CPPFLAGS} -DGIT_USE_ICONV"
fi

# Add definitions specific for solaris
case "${host_os}" in
    solaris*)
        CPPFLAGS="${CPPFLAGS} -D_POSIX_C_SOURCE=200112L -D__EXTENSIONS__ -D_POSIX_PTHREAD_SEMANTICS"
    ;;
esac

# Determine architecture of the machine
AC_CHECK_SIZEOF([void*])
if test "x$ac_cv_sizeof_voidp" == "x8"; then
    CPPFLAGS="${CPPFLAGS} -DGIT_ARCH_64"
elif test "x$ac_cv_sizeof_voidp" == "x4"; then
    CPPFLAGS="${CPPFLAGS} -DGIT_ARCH_32"
else
    AC_MSG_FAILURE([Unsupported architecture])
fi

# Checks for structures
AC_CHECK_MEMBERS([struct stat.st_atimespec],
                 [CPPFLAGS="${CPPFLAGS} -DGIT_USE_STAT_ATIMESPEC"],
                 [],
                 [#ifdef HAVE_SYS_TYPES_H
                  # include <sys/types.h>
                  #endif
                  #ifdef HAVE_SYS_STAT_H
                  # include <sys/stat.h>
                  #endif])

# Checks for library functions.
AC_CHECK_FUNCS([futimens qsort_r qsort_s])

if test $ac_cv_func_futimens = yes; then
    CPPFLAGS="${CPPFLAGS} -DHAVE_FUTIMENS"
fi

if test $ac_cv_func_qsort_r = yes; then
    CPPFLAGS="${CPPFLAGS} -DHAVE_QSORT_R"
fi

if test $ac_cv_func_qsort_s = yes; then
    CPPFLAGS="${CPPFLAGS} -DHAVE_QSORT_S"
fi

# Check if compiler flag '-Wno-unused-function' is supported
oCFLAGS="${CFLAGS}"
CFLAGS="-Wno-unused-function"
AC_MSG_CHECKING([if '${CFLAGS}' is supported])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],[])],
	          [AC_MSG_RESULT([yes])]
		  PKG_CFLAGS="${PKG_CFLAGS} ${CFLAGS}",
		  [AC_MSG_RESULT([no])])
CFLAGS="${oCFLAGS}"

AC_SUBST(GIT2R_SRC_REGEX)
AC_SUBST([PKG_CFLAGS], ["${PKG_CFLAGS} ${LIBSSH2_CFLAGS}"])
AC_SUBST([PKG_CPPFLAGS], ["${CPPFLAGS} ${LIBCURL_CPPFLAGS}"])
AC_SUBST([PKG_LIBS], ["${LIBS} ${LIBSSH2_LIBS} ${LIBCURL} ${LIBICONV}"])

AC_CONFIG_FILES([src/Makevars])

echo "
  ----- Results of the git2r package configure -----

    OpenSSL to talk over HTTPS...........: ${have_ssl}
    LibSSH2 to enable the SSH transport..: ${have_ssh2}

  --------------------------------------------------
"

AC_OUTPUT
