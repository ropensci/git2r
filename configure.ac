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
AC_INIT([git2r], [see.DESCRIPTION.file], [https://github.com/ropensci/git2r/issues])
AC_CONFIG_SRCDIR([src/git2r.c])
AC_CONFIG_AUX_DIR([tools])

AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CC

## check for pkg-config
AC_DEFUN([AC_PROG_PKGCONFIG], [AC_CHECK_PROG(PKGCONFIG,pkg-config,yes)])
AC_PROG_PKGCONFIG

# Handle optional include and library paths
GIT2R_CPPFLAGS="-D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -I. -Ilibgit2 -Ilibgit2/include -Ihttp-parser"

# Include and use regex on solaris
case "${host_os}" in
  solaris*)
    GIT2R_CPPFLAGS="${GIT2R_CPPFLAGS} -Iregex"
    GIT2R_SRC_REGEX=regex/regex.o
    ;;
esac

# zlib include and library path
AC_ARG_WITH([zlib-include],
    AC_HELP_STRING([--with-zlib-include=INCLUDE_PATH],
                   [the location of zlib header files]),
    [zlib_include_path=$withval])
if test [ -n "$zlib_include_path" ] ; then
  GIT2R_CPPFLAGS="${GIT2R_CPPFLAGS} -I${zlib_include_path}"
fi

AC_ARG_WITH([zlib-lib],
    AC_HELP_STRING([--with-zlib-lib=LIB_PATH],
                   [the location of zlib libraries]),
    [zlib_lib_path=$withval])
if test [ -n "$zlib_lib_path" ] ; then
  LIBS="-L${zlib_lib_path} ${LIBS}"
fi

# LibSSH2 include and library path
AC_ARG_WITH([libssh2-include],
    AC_HELP_STRING([--with-libssh2-include=INCLUDE_PATH],
                   [the location of LibSSH2 header files]),
    [libssh2_include_path=$withval])
if test [ -n "$libssh2_include_path" ] ; then
  GIT2R_CPPFLAGS="${GIT2R_CPPFLAGS} -I${libssh2_include_path}"
fi

AC_ARG_WITH([libssh2-lib],
    AC_HELP_STRING([--with-libssh2-lib=LIB_PATH],
                   [the location of LibSSH2 libraries]),
    [libssh2_lib_path=$withval])
if test [ -n "$libssh2_lib_path" ] ; then
  LIBS="-L${libssh2_lib_path} ${LIBS}"
fi

# libssl include and library path
AC_ARG_WITH([libssl-include],
    AC_HELP_STRING([--with-libssl-include=INCLUDE_PATH],
                   [the location of libssl header files]),
    [libssl_include_path=$withval])
if test [ -n "$libssl_include_path" ] ; then
  GIT2R_CPPFLAGS="${GIT2R_CPPFLAGS} -I${libssl_include_path}"
fi

AC_ARG_WITH([libssl-lib],
    AC_HELP_STRING([--with-libssl-lib=LIB_PATH],
                   [the location of libssl libraries]),
    [libssl_lib_path=$withval])
if test [ -n "$libssl_lib_path" ] ; then
  LIBS="-L${libssl_lib_path} ${LIBS}"
fi

# libcrypto include and library path
AC_ARG_WITH([libcrypto-include],
    AC_HELP_STRING([--with-libcrypto-include=INCLUDE_PATH],
                   [the location of libcrypto header files]),
    [libcrypto_include_path=$withval])
if test [ -n "$libcrypto_include_path" ] ; then
  GIT2R_CPPFLAGS="${GIT2R_CPPFLAGS} -I${libcrypto_include_path}"
fi

AC_ARG_WITH([libcrypto-lib],
    AC_HELP_STRING([--with-libcrypto-lib=LIB_PATH],
                   [the location of libcrypto libraries]),
    [libcrypto_lib_path=$withval])
if test [ -n "$libcrypto_lib_path" ] ; then
  LIBS="-L${libcrypto_lib_path} ${LIBS}"
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
CPPFLAGS="${GIT2R_CPPFLAGS} ${CPPFLAGS}"

# Check for zlib
have_zlib=no
AC_SEARCH_LIBS([inflate], [z], [have_zlib=yes])

if test "x${have_zlib}" = xno; then
        AC_MSG_FAILURE([
  ---------------------------------------------
   zlib library required

   Please install:
   zlib1g-dev (deb) or zlib-devel (rpm)
  ---------------------------------------------])
fi

# Check for SSL
case "${host_os}" in
    darwin*)
        # On OS X, use Security and CoreFoundation framework
        CPPFLAGS="${CPPFLAGS} ${SECURITY_INCLUDE_DIR} -DGIT_SECURE_TRANSPORT -DGIT_COMMON_CRYPTO"
        LIBS="${LIBS} -framework Security -framework CoreFoundation"
        GIT2R_SRC_LIBGIT2_STREAM=libgit2/stransport_stream.o
    ;;
    *)
        # Check for OpenSSL
        have_ssl=no
        if test "x${OPENSSL_INCLUDES}" = x; then :; else
            CPPFLAGS="${CPPFLAGS} -I${OPENSSL_INCLUDES}"
        fi
        AC_SEARCH_LIBS([EVP_EncryptInit], [crypto],
        [
            AC_SEARCH_LIBS([SSL_library_init], [ssl], [have_ssl=yes])
        ])

        # Specify sha1 implementation
        if test "x${have_ssl}" = xyes; then
            CPPFLAGS="${CPPFLAGS} -DGIT_OPENSSL -DOPENSSL_SHA1"
        else
            AC_MSG_FAILURE([
  ---------------------------------------------
   OpenSSL library required

   Please install:
   libssl-dev (deb) or openssl-devel (rpm)
  ---------------------------------------------])
        fi
    ;;
esac

# Check for LibSSH2
have_ssh2=no
if test "x${PKGCONFIG}" = xyes; then
    AC_MSG_CHECKING([whether pkg-config could find libssh2])
    if pkg-config --exists libssh2; then
        AC_MSG_RESULT([yes])
        libssh2_cppflags=`pkg-config --cflags libssh2`
        GIT2R_LIBSSH2=`pkg-config --libs libssh2`
        CPPFLAGS="${libssh2_cppflags} ${CPPFLAGS} -DGIT_SSH"
        have_ssh2=yes
    fi
else
    AC_MSG_WARN([
  ---------------------------------------------
   Unable to find pkg-config on this system
   to search for LibSSH2.

   To build with SSH support, please install:
   pkg-config
  ---------------------------------------------])
fi

if test "x${have_ssh2}" = xno; then
    # Static linking of LibSSH2 on OSX to support redistributable binary packages
    case "${host_os}" in
        darwin*)
            # Check for Homebrew in path
            case "${PATH}" in
                *brew*)
                    BREW_OPENSSL=`brew --prefix openssl`
                    CPPFLAGS="${CPPFLAGS} -I${BREW_OPENSSL}/include"
                    LDFLAGS="${LDFLAGS} -L${BREW_OPENSSL}/lib"
                ;;
            esac

            # Download and build libssh2
            mkdir -p libssh2 && curl -fsSL https://github.com/libssh2/libssh2/releases/download/libssh2-1.6.0/libssh2-1.6.0.tar.gz | tar xz --strip 1 -C libssh2
            mkdir -p src/libssh2
            GIT2R_LIBSSH2_INST_DIR="${PWD}/src/libssh2"
            export CPPFLAGS
            export LDFLAGS
            cp -f tools/missing libssh2
            cd libssh2
            ./configure --with-pic --disable-examples-build --disable-shared --enable-static --prefix=$GIT2R_LIBSSH2_INST_DIR \
                && make && make install && have_ssh2=yes
            cd ..

            if test "x${have_ssh2}" = xyes; then
                have_ssh2=no
                if test "x${PKGCONFIG}" = xyes; then
                    AC_MSG_CHECKING([whether pkg-config could find libssh2])
                    export=PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:${GIT2R_LIBSSH2_INST_DIR}/lib/pkgconfig"
                    export PKG_CONFIG_PATH
                    if pkg-config --exists libssh2; then
                        AC_MSG_RESULT([yes])
                        GIT2R_LIBSSH2=`pkg-config --libs --static libssh2`
                        CPPFLAGS="${CPPFLAGS} -DGIT_SSH -Ilibssh2/include"
                        have_ssh2=yes
                    fi
                fi
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
case "${host_os}" in
  darwin*)
        have_iconv=no
        AC_SEARCH_LIBS([iconv_open], [iconv], [have_iconv=yes])

        if test "x${have_iconv}" = xyes; then
                CPPFLAGS="${CPPFLAGS} -DGIT_USE_ICONV"
        else
                AC_MSG_FAILURE([iconv library required])
        fi
    ;;
esac

# Configuration specific for solaris
case "${host_os}" in
  solaris*)
    CPPFLAGS="${CPPFLAGS} -D_POSIX_C_SOURCE=200112L -D__EXTENSIONS__ -D_POSIX_PTHREAD_SEMANTICS"
    AC_SEARCH_LIBS(connect, [socket])
    AC_SEARCH_LIBS(gethostbyname, [nsl socket])
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

AC_SUBST(GIT2R_SRC_LIBGIT2_STREAM)
AC_SUBST(GIT2R_SRC_REGEX)
AC_SUBST(GIT2R_LIBSSH2)
AC_SUBST(CPPFLAGS)
AC_SUBST(LIBS)

AC_CONFIG_FILES([src/Makevars])

AC_OUTPUT
