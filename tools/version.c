#include <git2.h>

#if LIBGIT2_VER_MAJOR == 0
#    if LIBGIT2_VER_MINOR < 26
#        error libgit2 version too old
#    endif
#elif LIBGIT2_VER_MAJOR > 1
#    error the libgit2 version is not compatible with git2r
#endif
