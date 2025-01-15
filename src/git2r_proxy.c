#include "git2r_proxy.h"
#include <stdlib.h>
#include <string.h>

/**
 * Initialize and set libgit2 proxy options from an R object, storing
 * the allocated proxy URL in a payload struct.
 *
 * This function populates \p proxy_opts based on \p proxy_val:
 * \li If \p proxy_val is \c R_NilValue (i.e., NULL in R), proxy is
 *     set to \c GIT_PROXY_NONE (disabled).
 * \li If \p proxy_val is a logical \c TRUE, proxy is set to
 *     \c GIT_PROXY_AUTO (automatic detection).
 * \li If \p proxy_val is a character vector of length 1, proxy is
 *     set to \c GIT_PROXY_SPECIFIED with that URL.
 * Otherwise, the function returns \c -1 to indicate invalid
 * or unsupported input.
 *
 * @param proxy_opts A pointer to a \c git_proxy_options struct, which
 *                   is initialized via \c git_proxy_options_init
 *                   and then populated according to \p proxy_val.
 * @param proxy_val  An R object specifying the desired proxy
 *                   configuration: \c NULL (no proxy), a logical
 *                   \c TRUE (auto detection), or a character string
 *                   (URL).
 * @return           0 on success, or -1 if \p proxy_val is invalid
 *                   or memory allocation fails.
 *
 * \note The caller is responsible for calling
 *       \c git2r_proxy_payload_free() on the \p payload struct
 *       to deallocate the proxy URL after \c git_remote_connect()
 *       or \c git_remote_fetch() has finished using it.
 */
int git2r_set_proxy_options(git_proxy_options *proxy_opts, SEXP proxy_val)
{
    git_proxy_options_init(proxy_opts, GIT_PROXY_OPTIONS_VERSION);
    proxy_opts->type = GIT_PROXY_NONE;
    proxy_opts->url = NULL;

    if (Rf_isNull(proxy_val)) {
        return 0; /* No proxy */
    } else if (Rf_isLogical(proxy_val) && LOGICAL(proxy_val)[0]) {
        proxy_opts->type = GIT_PROXY_AUTO; /* Auto-detect proxy */
        return 0;
    } else if (Rf_isString(proxy_val) && Rf_length(proxy_val) == 1) {
        const char *url = CHAR(STRING_ELT(proxy_val, 0));
        if (url) {
            proxy_opts->type = GIT_PROXY_SPECIFIED;
            proxy_opts->url = url;
        }
        return 0;
    }
    return -1; /* Invalid input */
}
