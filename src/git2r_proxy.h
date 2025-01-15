#ifndef GIT2R_PROXY_H
#define GIT2R_PROXY_H

#include <Rinternals.h>
#include <git2.h>

/* Function to initialize proxy options */
int git2r_set_proxy_options(git_proxy_options *proxy_opts, SEXP proxy_val);

#endif /* GIT2R_PROXY_H */
