# git2r, R bindings to the libgit2 library.
# Copyright (C) 2013-2018 The git2r contributors
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

## Check for brew on macOS
AC_DEFUN([AC_PROG_BREW], [AC_CHECK_PROG(BREW,brew,yes)])

# Use pkg-config if available
PKG_PROG_PKG_CONFIG
if test [ -n "$PKG_CONFIG" ] ; then
    PKG_CHECK_MODULES([libgit2], [libgit2],
                      [PKGCONFIG_CFLAGS="${libgit2_CFLAGS}"
                       PKGCONFIG_LIBS="${libgit2_LIBS}"],
                      [USE_BUNDLED_LIBGIT2=yes])
fi

# Find the compiler and compiler flags to use
: ${R_HOME=`R RHOME`}
if test -z "${R_HOME}"; then
    AC_MSG_FAILURE([Could not determine R_HOME])
fi
RBIN="${R_HOME}/bin/R"
CC=`"${RBIN}" CMD config CC`
CFLAGS=`"${RBIN}" CMD config CFLAGS`
CPPFLAGS=`"${RBIN}" CMD config CPPFLAGS`

USE_BUNDLED_LIBGIT2=yes
if test "x${INCLUDE_DIR}${LIB_DIR}" = x; then
    if test "x${PKGCONFIG_CFLAGS}${PKGCONFIG_LIBS}" = x; then
        case "${host_os}" in
            darwin*)
                AC_PROG_BREW
                if test "x${BREW}" = xyes; then
                    BREWDIR=`brew --prefix`
                fi
                CFLAGS="${CFLAGS} -I${BREWDIR}/opt/libgit2/include"
                LIBS="-L${BREWDIR}/opt/libgit2/lib"
                USE_BUNDLED_LIBGIT2=no
            ;;
        esac
    else
        echo "Found pkg-config cflags and libs!"
        CFLAGS="${CFLAGS} ${PKGCONFIG_CFLAGS}"
        LIBS="${PKGCONFIG_LIBS}"
        USE_BUNDLED_LIBGIT2=no
    fi
else
    echo "Found INCLUDE_DIR and/or LIB_DIR!"
    CFLAGS="-I${INCLUDE_DIR} ${CFLAGS}"
    LIBS="-L${LIB_DIR}"
    USE_BUNDLED_LIBGIT2=no,
fi

################# Begin configuration to use system libgit2 ##################
if test "x${USE_BUNDLED_LIBGIT2}" = xno; then
    AC_MSG_CHECKING([whether the libgit2 version will work in git2r])
    libgit2_ver_ok=no
    ${CC} ${CPPFLAGS} ${CFLAGS} -E tools/version.c >/dev/null 2>&1 && libgit2_ver_ok=yes
    AC_MSG_RESULT([${libgit2_ver_ok}])

    if test "x${libgit2_ver_ok}" = xno; then
        AC_MSG_FAILURE([
  ---------------------------------------------
   The libgit2 library that is required to build git2r was not found.

   Please install:
     libgit2-dev   (package on e.g. Debian and Ubuntu)
     libgit2-devel (package on e.g. Fedora, CentOS and RHEL)
     libgit2       (Homebrew package on macOS)
   and try again.

   If the libgit2 library is installed on your system but the git2r configuration is
   unable to find it, you can specify the include and lib path to libgit2 with:
   R CMD INSTALL git2r --configure-vars='INCLUDE_DIR=path/to/include LIB_DIR=path/to/lib'
  ---------------------------------------------])
    fi

    AC_SUBST([PKG_CFLAGS], ["${CFLAGS}"])
    AC_SUBST([PKG_LIBS], ["${LIBS}"])
    AC_CONFIG_FILES([src/Makevars])
    AC_OUTPUT
fi
################# End configuration to use system libgit2 ####################

################# Begin configuration to build bundled libgit2 ###############
if test "x${USE_BUNDLED_LIBGIT2}" = xyes; then

AC_MSG_NOTICE([Attempting configuration of bundled libgit2])

# Use R to determine architecture of the machine
AC_MSG_CHECKING([size of void*])
sizeof_voidp=`"${RBIN}" --slave --vanilla -e "cat(.Machine\\$sizeof.pointer)"`
AC_MSG_RESULT([$sizeof_voidp])
if test "x$sizeof_voidp" = "x8"; then
    CPPFLAGS="${CPPFLAGS} -DGIT_ARCH_64"
elif test "x$sizeof_voidp" = "x4"; then
    CPPFLAGS="${CPPFLAGS} -DGIT_ARCH_32"
else
    AC_MSG_FAILURE([Unsupported architecture])
fi

# Check for zlib
have_zlib=no

AC_SEARCH_LIBS([inflate], [z], [have_zlib=yes])

if test "x${have_zlib}" = xno; then
    if test [ -n "$PKG_CONFIG" ] ; then
        PKG_CHECK_MODULES([zlib], [zlib],
                          [CPPFLAGS="${zlib_CFLAGS} ${CPPFLAGS}"
                           LIBS="${zlib_LIBS} ${LIBS}"
                           have_zlib=yes], [ ])
    fi
fi

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
   R CMD INSTALL git2r --configure-vars='LIBS=-L/path/to/libs CPPFLAGS=-I/path/to/headers'
  ---------------------------------------------])
fi

# Check for SSL
have_ssl=no

if test "x${OPENSSL_INCLUDES}" = x; then :; else
    CPPFLAGS="${CPPFLAGS} -I${OPENSSL_INCLUDES}"
fi

AC_SEARCH_LIBS([EVP_EncryptInit],
               [crypto],
               [AC_SEARCH_LIBS([SSL_library_init], [ssl], [have_ssl=yes])
                AC_SEARCH_LIBS([OPENSSL_init_ssl], [ssl], [have_ssl=yes])])

if test "x${have_ssl}" = xno; then
    if test [ -n "$PKG_CONFIG" ] ; then
        PKG_CHECK_MODULES([openssl], [openssl],
                          [CPPFLAGS="${openssl_CFLAGS} ${CPPFLAGS}"
                           LIBS="${openssl_LIBS} ${LIBS}"
                           have_ssl=yes], [ ])
    fi
fi

if test "x${have_ssl}" = xno; then
    AC_MSG_WARN([
  ---------------------------------------------
   Unable to find the OpenSSL library on this
   system. Building a version without support
   for HTTPS transport.

   Please install:
     libssl-dev    (package on e.g. Debian and Ubuntu)
     openssl-devel (package on e.g. Fedora, CentOS and RHEL)
   and try again.

   If the OpenSSL library is installed on
   your system but the git2r configuration
   is unable to find it, you can specify
   the include and lib path to OpenSSL with:
   R CMD INSTALL git2r --configure-vars='LIBS=-L/path/to/libs CPPFLAGS=-I/path/to/headers'
  ---------------------------------------------])
fi

# Check for LibSSH2
have_ssh2=no
AC_SEARCH_LIBS([libssh2_version], [ssh2], [have_ssh2=yes])

if test "x${have_ssh2}" = xno; then
    if test [ -n "$PKG_CONFIG" ] ; then
        PKG_CHECK_MODULES([libssh2], [libssh2],
                          [CPPFLAGS="${libssh2_CFLAGS} ${CPPFLAGS}"
                           LIBS="${libssh2_LIBS} ${LIBS}"
                           have_ssh2=yes], [ ])
    fi
fi

if test "x${have_ssh2}" = xno; then
    AC_MSG_WARN([
  ---------------------------------------------
   Unable to find the LibSSH2 library on this
   system. Building a version without support
   for SSH transport.

   To build with SSH support, please install:
     libssh2-1-dev (package on e.g. Debian and Ubuntu)
     libssh2-devel (package on e.g. Fedora, CentOS and RHEL)
   and try again.

   If the LibSSH2 library is installed on
   your system but the git2r configuration
   is unable to find it, you can specify
   the include and lib path to LibSSH2 with:
   R CMD INSTALL git2r --configure-vars='LIBS=-L/path/to/libs CPPFLAGS=-I/path/to/headers'
  ---------------------------------------------])
fi

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
CPPFLAGS="${CPPFLAGS} -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -DLIBGIT2_NO_FEATURES_H -DR_NO_REMAP -DSTRICT_R_HEADERS"

# Specify sha1 implementation
CPPFLAGS="${CPPFLAGS} -DGIT_SHA1_COLLISIONDETECT -DSHA1DC_NO_STANDARD_INCLUDES=1"
CPPFLAGS="${CPPFLAGS} -DSHA1DC_CUSTOM_INCLUDE_SHA1_C=\\\"common.h\\\" -DSHA1DC_CUSTOM_INCLUDE_UBC_CHECK_C=\\\"common.h\\\""

# Add definition for https
if test "x${have_ssl}" = xyes; then
    CPPFLAGS="${CPPFLAGS} -DGIT_OPENSSL -DGIT_HTTPS=1"
fi

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

# Checks for structures
AC_CHECK_MEMBER([struct stat.st_mtim],
                [CPPFLAGS="${CPPFLAGS} -DGIT_USE_STAT_MTIM"],
                [], [AC_INCLUDES_DEFAULT])

AC_CHECK_MEMBER([struct stat.st_mtimespec],
                [CPPFLAGS="${CPPFLAGS} -DGIT_USE_STAT_MTIMESPEC"],
                [], [AC_INCLUDES_DEFAULT])

AC_CHECK_MEMBER([struct stat.st_mtime_nsec],
                [CPPFLAGS="${CPPFLAGS} -DGIT_USE_STAT_MTIME_NSEC"],
                [], [AC_INCLUDES_DEFAULT])

if test "x$ac_cv_member_struct_stat_st_mtim" = "xyes"; then
    AC_CHECK_MEMBER([struct stat.st_mtim.tv_nsec],
                    [CPPFLAGS="${CPPFLAGS} -DGIT_USE_NSEC"],
                    [], [AC_INCLUDES_DEFAULT])
elif test "x$ac_cv_member_struct_stat_st_mtimespec" = "xyes"; then
    AC_CHECK_MEMBER([struct stat.st_mtimespec.tv_nsec],
                    [CPPFLAGS="${CPPFLAGS} -DGIT_USE_NSEC"],
                    [], [AC_INCLUDES_DEFAULT])
else
    CPPFLAGS="${CPPFLAGS} -DGIT_USE_NSEC"
fi

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

AC_SUBST(GIT2R_SRC_REGEX)
AC_SUBST([PKG_CFLAGS], ["${PKG_CFLAGS} ${LIBSSH2_CFLAGS}"])
AC_SUBST([PKG_CPPFLAGS], ["${CPPFLAGS} ${LIBCURL_CPPFLAGS}"])
AC_SUBST([PKG_LIBS], ["${LIBS} ${LIBSSH2_LIBS} ${LIBCURL} ${LIBICONV}"])

AC_CONFIG_FILES([src/Makevars_libgit2])

echo "
  ----- Results of the git2r package configure -----

    OpenSSL to talk over HTTPS...........: ${have_ssl}
    LibSSH2 to enable the SSH transport..: ${have_ssh2}

  --------------------------------------------------
"

AC_OUTPUT

cd src; mv Makevars_libgit2 Makevars

fi
################# End configuration to build bundled libgit2 #################
