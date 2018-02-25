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

# Check for pkg-config
PKG_PROG_PKG_CONFIG

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
CPICFLAGS=`"${RBIN}" CMD config CPICFLAGS`

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
   R CMD INSTALL --configure-args='--with-zlib-include=INCLUDE_PATH --with-zlib-lib=LIB_PATH' git2r
  ---------------------------------------------])
fi

# Check for SSL
have_ssl=no

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

case "${host_os}" in
    darwin*)
        # Save flags
        oCPPFLAGS="${CPPFLAGS}"
        oLIBS="${LIBS}"

        # On OS X, try to find libssh2 in brew
        if test "x${BREW}" = xyes; then
            CPPFLAGS="${CPPFLAGS} -I${BREW_DIR}/opt/libssh2/include"
            LIBS="${LIBS} -L${BREW_DIR}/opt/libssh2/lib"
            AC_SEARCH_LIBS([libssh2_version], [ssh2], [have_ssh2=yes])
        fi

        if test "x${have_ssh2}" = xno; then
            CPPFLAGS="${oCPPFLAGS}"
            LIBS="${oLIBS}"

            AC_MSG_NOTICE([Download and build libssh2])

            mkdir -p libssh2 && \
            curl -fsSL https://www.libssh2.org/download/libssh2-1.8.0.tar.gz | \
            tar xz --strip 1 -C libssh2

            mkdir -p src/libgit2/deps/libssh2
            LIBSSH2_INST_DIR="${PWD}/src/libgit2/deps/libssh2"
            cp -f tools/missing libssh2
            cd libssh2
            ./configure --with-pic --with-openssl --disable-examples-build \
              --disable-shared --enable-static --prefix=$LIBSSH2_INST_DIR \
              CC="${CC}" CFLAGS="${CFLAGS} ${CPICFLAGS} -Wno-deprecated-declarations" \
              CPPFLAGS="${CPPFLAGS}" && make && make install && have_ssh2=yes
            cd ..
            rm -rf libssh2

            if test "x${have_ssh2}" = xyes; then
                LIBSSH2_CFLAGS="-Ilibgit2/deps/libssh2/include"
                LIBSSH2_LIBS="-Llibgit2/deps/libssh2/lib -lssh2"
            fi
        fi
    ;;
    *)
        AC_SEARCH_LIBS([libssh2_version], [ssh2], [have_ssh2=yes])
    ;;
esac

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
CPPFLAGS="${CPPFLAGS} -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -DLIBGIT2_NO_FEATURES_H -DR_NO_REMAP -DSTRICT_R_HEADERS"

# Specify sha1 implementation
CPPFLAGS="${CPPFLAGS} -DGIT_SHA1_COLLISIONDETECT -DSHA1DC_NO_STANDARD_INCLUDES=1"
CPPFLAGS="${CPPFLAGS} -DSHA1DC_CUSTOM_INCLUDE_SHA1_C=\\\"common.h\\\" -DSHA1DC_CUSTOM_INCLUDE_UBC_CHECK_C=\\\"common.h\\\""

# Add definition for ssl
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

AC_CONFIG_FILES([src/Makevars])

echo "
  ----- Results of the git2r package configure -----

    OpenSSL to talk over HTTPS...........: ${have_ssl}
    LibSSH2 to enable the SSH transport..: ${have_ssh2}

  --------------------------------------------------
"

AC_OUTPUT
