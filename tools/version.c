#include <git2.h>

#if LIBGIT2_VER_MAJOR == 0
#    if LIBGIT2_VER_MINOR < 26
#        error libgit2 version too old
#    endif
#else
#    error the libgit2 version is not compatible with git2r
#endif
