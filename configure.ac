# git2r, R bindings to the libgit2 library.
# Copyright (C) 2013-2015 The git2r contributors
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

# Checks for programs.
AC_PROG_CC

## check for pkg-config
AC_DEFUN([AC_PROG_PKGCONFIG], [AC_CHECK_PROG(PKGCONFIG,pkg-config,yes)])
AC_PROG_PKGCONFIG

## check for brew on OS X
AC_DEFUN([AC_PROG_BREW], [AC_CHECK_PROG(BREW,brew,yes)])
case "${host_os}" in
    darwin*)
        AC_PROG_BREW
    ;;
esac

# Handle optional include and library paths
GIT2R_CPPFLAGS=

# iconv include and library path
AC_ARG_WITH([iconv-include],
            AC_HELP_STRING([--with-iconv-include=INCLUDE_PATH],
                           [the location of the iconv header files]),
            [iconv_include_path=$withval])

AC_ARG_WITH([iconv-lib],
            AC_HELP_STRING([--with-iconv-lib=LIB_PATH],
                           [the location of the iconv library]),
            [iconv_lib_path=$withval])

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
if test [ -n "$libssh2_include_path" ] ; then
  GIT2R_CPPFLAGS="${GIT2R_CPPFLAGS} -I${libssh2_include_path}"
fi

AC_ARG_WITH([libssh2-lib],
    AC_HELP_STRING([--with-libssh2-lib=LIB_PATH],
                   [the location of the LibSSH2 library]),
    [libssh2_lib_path=$withval])
if test [ -n "$libssh2_lib_path" ] ; then
  LIBS="-L${libssh2_lib_path} ${LIBS}"
fi

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
R_CMD_config_CC=`"${RBIN}" CMD config CC`
CC="${R_CMD_config_CC}"
R_CMD_config_CFLAGS=`"${RBIN}" CMD config CFLAGS`
CFLAGS="${R_CMD_config_CFLAGS}"
R_CMD_config_CPPFLAGS=`"${RBIN}" CMD config CPPFLAGS`
CPPFLAGS="${GIT2R_CPPFLAGS} ${R_CMD_config_CPPFLAGS}"
R_CMD_config_LDFLAGS=`"${RBIN}" CMD config LDFLAGS`
LDFLAGS="${R_CMD_config_LDFLAGS}"
R_CMD_config_CPICFLAGS=`"${RBIN}" CMD config CPICFLAGS`
CPICFLAGS="${R_CMD_config_CPICFLAGS}"

# Check for zlib
have_zlib=no

# Check for optional zlib paths
if test [ -n "$zlib_include_path" ] ; then
    ZLIB_CFLAGS="-I${zlib_include_path}"
    ZLIB_LIBS="-L${zlib_lib_path} -lz"
    have_zlib=yes
elif test [ -n "$zlib_lib_path" ] ; then
    ZLIB_CFLAGS="-I${zlib_include_path}"
    ZLIB_LIBS="-L${zlib_lib_path} -lz"
    have_zlib=yes
fi

if test "x${have_zlib}" = xno; then
    # Check for zlib with pkg-config

    if test "x${PKGCONFIG}" = xyes; then
        AC_MSG_CHECKING([whether pkg-config could find zlib])
        if pkg-config --exists zlib; then
            AC_MSG_RESULT([yes])
            ZLIB_CFLAGS=`pkg-config --cflags zlib`
            ZLIB_LIBS=`pkg-config --libs zlib`
            have_zlib=yes
        else
            AC_MSG_RESULT([no])
        fi
    fi
fi

if test "x${have_zlib}" = xno; then
    # Check for zlib with autoconf macro

    AC_SEARCH_LIBS([inflate], [z], [have_zlib=yes])
fi

if test "x${have_zlib}" = xno; then
        AC_MSG_FAILURE([
  ---------------------------------------------
   zlib library required

   Please install:
     zlib1g-dev (deb)
     zlib-devel (rpm)

   You can specify the include and lib path with:
   R CMD INSTALL --configure-args='--with-zlib-include=INCLUDE_PATH --with-zlib-lib=LIB_PATH' git2r
  ---------------------------------------------])
fi

# Check for SSL
have_ssl=no
test_build_ssl_pkg=no

# Check for optional libssl paths
if test [ -n "$libssl_include_path" ] ; then
    LIBSSL_CFLAGS="-I${libssl_include_path}"
    LIBSSL_LIBS="-L${libssl_lib_path} -lssl -lcrypto"
    test_build_ssl_pkg=yes
elif test [ -n "$libssl_lib_path" ] ; then
    LIBSSL_CFLAGS="-I${libssl_include_path}"
    LIBSSL_LIBS="-L${libssl_lib_path} -lssl -lcrypto"
    test_build_ssl_pkg=yes
fi

if test "x${test_build_ssl_pkg}" = xno; then
    # On OS X, check for libssl in brew

    case "${host_os}" in
        darwin*)
            if test "x${BREW}" = xyes; then
                BREW_DIR=`brew --prefix`
                LIBSSL_CFLAGS="-I${BREW_DIR}/opt/openssl/include"
                LIBSSL_LIBS="-L${BREW_DIR}/opt/openssl/lib -lssl -lcrypto"
                test_build_ssl_pkg=yes
            fi
        ;;
    esac
fi

if test "x${test_build_ssl_pkg}" = xno; then
    # Check for libssl with pkg-config

    if test "x${PKGCONFIG}" = xyes; then
        AC_MSG_CHECKING([whether pkg-config could find libssl])
        if pkg-config --exists libssl; then
            AC_MSG_RESULT([yes])
            LIBSSL_CFLAGS=`pkg-config --cflags libssl`
            LIBSSL_LIBS=`pkg-config --libs libssl`
            test_build_ssl_pkg=yes
        else
            AC_MSG_RESULT([no])
        fi
    fi
fi

if test "x${test_build_ssl_pkg}" = xyes; then
    # Check that build with OpenSSL will work in a package

    AC_MSG_CHECKING([whether OpenSSL will work in a package])
    AC_LANG_CONFTEST(
    [AC_LANG_PROGRAM(
    [[#include <openssl/ssl.h>]],
    [[ return SSL_library_init (); ]])])
    PKG_CFLAGS="${LIBSSL_CFLAGS}" PKG_LIBS="${LIBSSL_LIBS}" \
    "$RBIN" CMD SHLIB conftest.c 1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD && \
    "$RBIN" --vanilla -q -e "dyn.load(paste('conftest',.Platform\$dynlib.ext,sep=''))" \
    1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD && have_ssl=yes
    AC_MSG_RESULT([${have_ssl}])
fi

if test "x${have_ssl}" = xno; then
    # Check for libssl with autoconf macro

    if test "x${OPENSSL_INCLUDES}" = x; then :; else
        CPPFLAGS="${CPPFLAGS} -I${OPENSSL_INCLUDES}"
    fi

    AC_SEARCH_LIBS([EVP_EncryptInit],
                   [crypto],
                   [AC_SEARCH_LIBS([SSL_library_init],
                                   [ssl],
                                   [have_ssl=yes])])
fi

if test "x${have_ssl}" = xno; then
    AC_MSG_FAILURE([
  ---------------------------------------------
   OpenSSL library required

   Please install:
     libssl-dev    (deb)
     openssl-devel (rpm)
     openssl       (brew)

   You can specify the include and lib path with:
   R CMD INSTALL --configure-args='--with-libssl-include=INCLUDE_PATH --with-libssl-lib=LIB_PATH' git2r
  ---------------------------------------------])
fi

# Check for LibSSH2
have_ssh2=no
if test "x${PKGCONFIG}" = xyes; then
    AC_MSG_CHECKING([whether pkg-config could find libssh2])
    if pkg-config --exists libssh2; then
        AC_MSG_RESULT([yes])
        LIBSSH2_CFLAGS=`pkg-config --cflags libssh2`
        LIBSSH2_LIBS=`pkg-config --libs libssh2`
        have_ssh2=yes
    else
        AC_MSG_RESULT([no])
    fi
fi

if test "x${have_ssh2}" = xno; then
    AC_SEARCH_LIBS([libssh2_version], [ssh2], [have_ssh2=yes])
fi

if test "x${have_ssh2}" = xno; then
    # Static linking of LibSSH2 on OSX to support redistributable binary packages
    case "${host_os}" in
        darwin*)
            AC_MSG_NOTICE([Download and build libssh2])

            mkdir -p libssh2 && curl -fsSL \
                https://github.com/libssh2/libssh2/releases/download/libssh2-1.6.0/libssh2-1.6.0.tar.gz \
                | tar xz --strip 1 -C libssh2
            mkdir -p src/libssh2
            LIBSSH2_INST_DIR="${PWD}/src/libssh2"
            cp -f tools/missing libssh2
            cd libssh2
            ./configure --with-pic --with-openssl --disable-examples-build \
                --disable-shared --enable-static \
                --prefix=$LIBSSH2_INST_DIR CC="${CC}" \
                CFLAGS="${CFLAGS} ${CPICFLAGS} -Wno-deprecated-declarations" \
                CPPFLAGS="${CPPFLAGS}" LDFLAGS=${LDFLAGS} \
                && make && make install && have_ssh2=yes
            cd ..
            rm -rf libssh2

            if test "x${have_ssh2}" = xyes; then
                LIBSSH2_CFLAGS="-Ilibssh2/include"
                LIBSSH2_LIBS="-Llibssh2/lib -lssh2"
            fi
        ;;
    esac
fi

if test "x${have_ssh2}" = xno; then
    AC_MSG_WARN([
  ---------------------------------------------
   Unable to find LibSSH2 on this system.
   Building a version without support for SSH.

   To build with SSH support, please install:
   libssh2-1-dev (deb) or libssh2-devel (rpm)
  ---------------------------------------------])
fi

# Check for iconv
have_iconv=no
case "${host_os}" in
    darwin*)
        # Check for optional iconv include path
        if test [ -n "$iconv_include_path" ] ; then
            CPPFLAGS="-I${iconv_include_path} ${CPPFLAGS}"
        fi

        # Check for optional iconv lib path
        if test [ -n "$iconv_lib_path" ] ; then
            LIBS="-L${iconv_lib_path} ${LIBS}"
        fi

        AC_SEARCH_LIBS([iconv_open], [iconv], [have_iconv=yes])

        if test "x${have_iconv}" = xno; then
            AC_MSG_FAILURE([
  ---------------------------------------------
   iconv library required

   You can specify the include and lib path with:
   R CMD INSTALL --configure-args='--with-iconv-include=INCLUDE_PATH --with-iconv-lib=LIB_PATH' git2r
  ---------------------------------------------])
        fi
    ;;
esac

# Configuration specific for solaris
case "${host_os}" in
  solaris*)
    AC_SEARCH_LIBS(connect, [socket])
    AC_SEARCH_LIBS(gethostbyname, [nsl socket])

    # Include and use regex on solaris
    CPPFLAGS="-Iregex ${CPPFLAGS}"
    GIT2R_SRC_REGEX=regex/regex.o
    ;;
esac

# Add include paths for git2r
CPPFLAGS="-I. -Ilibgit2 -Ilibgit2/include -Ihttp-parser ${CPPFLAGS}"

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

# Add definitions for ssh
if test "x${have_ssh2}" = xyes; then
    CPPFLAGS="${CPPFLAGS} ${LIBSSH2_CFLAGS} -DGIT_SSH"
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

AC_SUBST(GIT2R_SRC_REGEX)
AC_SUBST(LIBSSH2_LIBS)
AC_SUBST([PKG_CFLAGS], ["${PKG_CFLAGS} ${ZLIB_CFLAGS} ${LIBSSL_CFLAGS} ${LIBSSH2_CFLAGS}"])
AC_SUBST(CPPFLAGS)
AC_SUBST([PKG_LIBS], ["${LIBS} ${ZLIB_LIBS} ${LIBSSL_LIBS} ${LIBSSH2_LIBS}"])

AC_CONFIG_FILES([src/Makevars])

echo "
  ------------------ Results of the git2r package configure -----------------

    *** User specified options ***
    with-libssh2-include....: ${libssh2_include_path}
    with-libssh2-lib........: ${libssh2_lib_path}

    *** R configure variables ***
    R CMD config CC.........: ${R_CMD_config_CC}
    R CMD config CFLAGS.....: ${R_CMD_config_CFLAGS}
    R CMD config CPPFLAGS...: ${R_CMD_config_CPPFLAGS}
    R CMD config LDFLAGS....: ${R_CMD_config_LDFLAGS}
    R CMD config CPICFLAGS..: ${R_CMD_config_CPICFLAGS}

    *** Build options ***
    ZLIB_CFLAGS.............: ${ZLIB_CFLAGS}
    ZLIB_LIBS...............: ${ZLIB_LIBS}
    LIBSSL_CFLAGS...........: ${LIBSSL_CFLAGS}
    LIBSSL_LIBS.............: ${LIBSSL_LIBS}
    LIBSSH2_CFLAGS..........: ${LIBSSH2_CFLAGS}
    LIBSSH2_LIBS............: ${LIBSSH2_LIBS}

  ---------------------------------------------------------------------------
"

AC_OUTPUT
