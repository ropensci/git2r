# git2r, R bindings to the libgit2 library.
# Copyright (C) 2013-2019 The git2r contributors
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

# The minimum version of libgit2 that is compatible with git2r. The
# version 0.26 is in all Fedora releases and at least the latest
# Ubuntu.
LIBGIT2_MIN_VERSION=0.26.0

# Check if building against bundled libgit2 or system libgit2
USE_BUNDLED_LIBGIT2=yes
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
                    USE_BUNDLED_LIBGIT2=no
                elif test "x${autobrew}" = xyes; then
                    curl -sfL "https://jeroen.github.io/autobrew/${PKG_BREW_NAME}" > autobrew
                    source autobrew
                    USE_BUNDLED_LIBGIT2=no
                fi
            ;;
        esac
    else
        echo "Found pkg-config cflags and libs!"
        PKG_CFLAGS="${PKGCONFIG_CFLAGS}"
        PKG_LIBS="${PKGCONFIG_LIBS}"
        USE_BUNDLED_LIBGIT2=no
    fi
else
    echo "Found INCLUDE_DIR and/or LIB_DIR!"
    PKG_CFLAGS="-I${INCLUDE_DIR} ${PKG_CFLAGS}"
    PKG_LIBS="-L${LIB_DIR} ${PKG_LIBS}"
    USE_BUNDLED_LIBGIT2=no
fi

# Find the compiler and compiler flags to use
CC=`"${RBIN}" CMD config CC`
CFLAGS=`"${RBIN}" CMD config CFLAGS`
CPPFLAGS=`"${RBIN}" CMD config CPPFLAGS`

################# Begin configuration to use system libgit2 ##################
if test "x${USE_BUNDLED_LIBGIT2}" = xno; then

    AC_MSG_CHECKING([whether the libgit2 version will work in git2r])
    libgit2_ver_ok=no
    ${CC} ${CPPFLAGS} ${PKG_CFLAGS} ${CFLAGS} -E tools/version.c >/dev/null 2>&1 && libgit2_ver_ok=yes
    AC_MSG_RESULT([${libgit2_ver_ok}])

    # The function 'git_buf_free' is deprecated in libgit2. Use
    # 'git_buf_dispose', if available, instead.
    AC_MSG_CHECKING([whether the libgit2 function git_buf_dispose is available])
    have_buf_dispose=no
    AC_LANG_CONFTEST(
    [AC_LANG_PROGRAM([[#include <git2.h>]], [[ git_buf_dispose(NULL); ]])])
    PKG_CFLAGS="${PKG_CFLAGS} -Werror" "$RBIN" CMD SHLIB conftest.c \
    1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD && have_buf_dispose=yes
    AC_MSG_RESULT([${have_buf_dispose}])
    if test "x${have_buf_dispose}" = xyes; then
        PKG_CFLAGS="${PKG_CFLAGS} -DGIT2R_HAVE_BUF_DISPOSE"
    fi

    # The constants GIT_OBJ_ANY, GIT_OBJ_BLOB, GIT_OBJ_COMMIT,
    # GIT_OBJ_TAG_GIT_OBJ_TREE and GIT_REF_OID are deprecated in
    # libgit2. Use GIT_OBJECT_ANY, GIT_OBJECT_BLOB, GIT_OBJECT_COMMIT,
    # GIT_OBJECT_TAG_GIT_OBJECT_TREE and GIT_REFERENCE_DIRECT, if
    # available, instead.
    AC_MSG_CHECKING([whether the libgit2 constant GIT_OBJECT_ANY is available])
    have_git_object_any=no
    AC_LANG_CONFTEST([AC_LANG_PROGRAM(
    [[#include <git2.h>]],
    [[git_object_typeisloose(GIT_OBJECT_ANY);]])])
    PKG_CFLAGS="${PKG_CFLAGS} -Werror" "$RBIN" CMD SHLIB conftest.c \
    1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD && have_git_object_any=yes
    AC_MSG_RESULT([${have_git_object_any}])
    if test "x${have_git_object_any}" = xyes; then
        PKG_CFLAGS="${PKG_CFLAGS} -DGIT2R_HAVE_OBJECT_ANY"
    fi

    # For debugging
    echo "----- Results of the git2r package configure -----"
    echo ""
    echo "  PKG_CFLAGS: ${PKG_CFLAGS}"
    echo "  PKG_LIBS: ${PKG_LIBS}"
    echo ""
    echo "--------------------------------------------------"

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

    AC_SUBST([PKG_CFLAGS], ["${PKG_CFLAGS}"])
    AC_SUBST([PKG_LIBS], ["${PKG_LIBS}"])
    AC_CONFIG_FILES([src/Makevars])

    AC_OUTPUT
fi
################# End configuration to use system libgit2 ####################

################# Begin configuration to build bundled libgit2 ###############
if test "x${USE_BUNDLED_LIBGIT2}" = xyes; then

AC_MSG_NOTICE([Package dependency requirement 'libgit2 >= ${LIBGIT2_MIN_VERSION}' could not be satisfied.])

echo "
  -----------------------------------------------------------------------

   Unable to find the libgit2 library on this system. Building 'git2r'
   using the bundled source of the libgit2 library.

   To build git2r with a system installation of libgit2, please install:
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

# Check for SSL for https transport
have_ssl=no
case "${host_os}" in
    darwin*)
        # On macOS, use the Security and CoreFoundation framework
        have_ssl=yes
        CPPFLAGS="${CPPFLAGS} -DGIT_SECURE_TRANSPORT=1 -DGIT_HTTPS=1"
        LIBS="${LIBS} -framework Security -framework CoreFoundation"
    ;;
    *)
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

        if test "x${have_ssl}" = xyes; then
            CPPFLAGS="${CPPFLAGS} -DGIT_OPENSSL=1 -DGIT_HTTPS=1"
        else
            AC_MSG_WARN([
  ---------------------------------------------
   Unable to find the OpenSSL library on this
   system. Building a version without support
   for https transport.

   To build with https support, please install:
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
    ;;
esac

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
     libssh2 (Homebrew package on OS X)
   and try again.

   If the LibSSH2 library is installed on
   your system but the git2r configuration
   is unable to find it, you can specify
   the include and lib path to LibSSH2 with:
   R CMD INSTALL git2r --configure-vars='LIBS=-L/path/to/libs CPPFLAGS=-I/path/to/headers'
  ---------------------------------------------])
else
    CPPFLAGS="${CPPFLAGS} -DGIT_SSH"
fi

# Check for iconv
case "${host_os}" in
    darwin*)
        AM_ICONV
        if test "x${am_cv_func_iconv}" = xyes; then
            CPPFLAGS="${CPPFLAGS} -DGIT_USE_ICONV"
        fi
    ;;
esac

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
CPPFLAGS="${CPPFLAGS} -DGIT_SHA1_COLLISIONDETECT=1 -DSHA1DC_NO_STANDARD_INCLUDES=1"
CPPFLAGS="${CPPFLAGS} -DSHA1DC_CUSTOM_INCLUDE_SHA1_C=\\\"common.h\\\" -DSHA1DC_CUSTOM_INCLUDE_UBC_CHECK_C=\\\"common.h\\\""

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

# The function 'git_buf_free' is deprecated in libgit2. Use
# 'git_buf_dispose' instead.
#
CPPFLAGS="${CPPFLAGS} -DGIT2R_HAVE_BUF_DISPOSE"

# The constants GIT_OBJ_ANY, GIT_OBJ_BLOB, GIT_OBJ_COMMIT,
# GIT_OBJ_TAG_GIT_OBJ_TREE and GIT_REF_OID are deprecated in
# libgit2. Use GIT_OBJECT_ANY, GIT_OBJECT_BLOB, GIT_OBJECT_COMMIT,
# GIT_OBJECT_TAG_GIT_OBJECT_TREE and GIT_REFERENCE_DIRECT instead.
CPPFLAGS="${CPPFLAGS} -DGIT2R_HAVE_OBJECT_ANY"

AC_SUBST(GIT2R_SRC_REGEX)
AC_SUBST([PKG_CFLAGS], ["${PKG_CFLAGS} ${LIBSSH2_CFLAGS}"])
AC_SUBST([PKG_CPPFLAGS], ["${CPPFLAGS}"])
AC_SUBST([PKG_LIBS], ["${LIBS} ${LIBSSH2_LIBS} ${LIBICONV}"])

AC_CONFIG_FILES([src/Makevars_libgit2])

echo "
  ----- Results of the git2r package configure -----

    HTTPS transport......................: ${have_ssl}
    LibSSH2 to enable the SSH transport..: ${have_ssh2}

  --------------------------------------------------
"

AC_OUTPUT

cd src; mv Makevars_libgit2 Makevars

fi
################# End configuration to build bundled libgit2 #################
