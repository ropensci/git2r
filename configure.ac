# git2r, R bindings to the libgit2 library.
# Copyright (C) 2013-2023 The git2r contributors
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

m4_include([tools/lib-ld.m4])
m4_include([tools/lib-link.m4])
m4_include([tools/lib-prefix.m4])
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

################# Begin configuration to use system libgit2 ##################
# Check for new sys subfolfder in libgit2.
AC_MSG_CHECKING([whether libgit2 includes new sys subfolder])
have_sys_subfolder=no
AC_LANG_CONFTEST([AC_LANG_PROGRAM(
[[#include <git2/sys/errors.h>]],
[[git_error_set_str(0, NULL);]])])
PKG_CFLAGS="${PKG_CFLAGS}" "$RBIN" CMD SHLIB conftest.c \
1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD && have_sys_subfolder=yes
AC_MSG_RESULT([${have_sys_subfolder}])
if test "x${have_sys_subfolder}" = xyes; then
    PKG_CFLAGS="${PKG_CFLAGS} -DGIT2R_HAVE_SYS_SUBFOLDER"
fi

AC_MSG_CHECKING([whether the libgit2 function git_buf_dispose is available])
have_buf_dispose=no
AC_LANG_CONFTEST([AC_LANG_PROGRAM(
[[#include <git2.h>]],
[[git_buf_dispose(NULL);]])])
PKG_CFLAGS="${PKG_CFLAGS} -Werror" "$RBIN" CMD SHLIB conftest.c \
1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD && have_buf_dispose=yes
AC_MSG_RESULT([${have_buf_dispose}])
if test "x${have_buf_dispose}" = xyes; then
    PKG_CFLAGS="${PKG_CFLAGS} -DGIT2R_HAVE_BUF_DISPOSE"
fi

# The constants GIT_OBJ_ANY, GIT_OBJ_BLOB, GIT_OBJ_COMMIT,
# GIT_OBJ_TAG_GIT_OBJ_TREE and GIT_REF_OID are deprecated in
# libgit2 v0.28.0. Use GIT_OBJECT_ANY, GIT_OBJECT_BLOB,
# GIT_OBJECT_COMMIT, GIT_OBJECT_TAG_GIT_OBJECT_TREE and
# GIT_REFERENCE_DIRECT, if available, instead.
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

# Several libgit2 error functions and enumaration values have been
# deprecated, use newer versions.
AC_MSG_CHECKING([whether the libgit2 function git_error_last is available])
have_git_error_last=no
AC_LANG_CONFTEST([AC_LANG_PROGRAM(
[[#include <git2.h>]],
[[git_error_last();]])])
PKG_CFLAGS="${PKG_CFLAGS} -Werror" "$RBIN" CMD SHLIB conftest.c \
1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD && have_git_error_last=yes
AC_MSG_RESULT([${have_git_error_last}])
if test "x${have_git_error_last}" = xyes; then
    PKG_CFLAGS="${PKG_CFLAGS} -DGIT2R_HAVE_GIT_ERROR"
fi

# libgit v0.99.0: Several structures, enums and values have been
# renamed in libgit version 0.99.0. The former names are
# deprecated.  See
# https://github.com/libgit2/libgit2/releases/tag/v0.99.0
AC_MSG_CHECKING([whether the libgit2 function git_oid_is_zero is available])
have_git_oid_is_zero=no
AC_LANG_CONFTEST([AC_LANG_PROGRAM(
[[#include <git2.h>]],
[[git_oid_is_zero(NULL);]])])
PKG_CFLAGS="${PKG_CFLAGS} -Werror" "$RBIN" CMD SHLIB conftest.c \
1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD && have_git_oid_is_zero=yes
AC_MSG_RESULT([${have_git_oid_is_zero}])
if test "x${have_git_oid_is_zero}" = xyes; then
    PKG_CFLAGS="${PKG_CFLAGS} -DGIT2R_LIBGIT2_V0_99_0_RENAMES"
fi

# libgit v1.8.0 requires code changes around functions taking
# git_commit.  See
# https://github.com/libgit2/libgit2/issues/6793
AC_MSG_CHECKING([whether libgit2 function requires cast to 'git_commit*const*'])
have_git_commit_const_pointer=no
AC_LANG_CONFTEST([AC_LANG_PROGRAM(
[[#include <git2.h>]],
[[
  git_oid* oid = NULL;
  git_signature *author = NULL;
  git_signature *committer = NULL;
  git_tree *tree = NULL;
  git_repository *repository = NULL;
  git_commit **parents = NULL;

  git_commit_create(
      oid,
      repository,
      "HEAD",
      author,
      committer,
      NULL,
      "",
      tree,
      0,
      (git_commit*const*)parents);
]])])
PKG_CFLAGS="${PKG_CFLAGS} -Werror" "$RBIN" CMD SHLIB conftest.c \
1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD && have_git_commit_const_pointer=yes
AC_MSG_RESULT([${have_git_commit_const_pointer}])
if test "x${have_git_commit_const_pointer}" = xyes; then
    PKG_CFLAGS="${PKG_CFLAGS} -DGIT2R_GIT_COMMIT_CONST_POINTER"
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
