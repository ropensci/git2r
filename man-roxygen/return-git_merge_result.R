##' @return A list of class \code{git_merge_result} with entries:
##' \describe{
##'   \item{up_to_date}{
##'     TRUE if the merge is already up-to-date, else FALSE.
##'   }
##'   \item{fast_forward}{
##'     TRUE if a fast-forward merge, else FALSE.
##'   }
##'   \item{conflicts}{
##'     TRUE if the index contain entries representing file conflicts,
##'     else FALSE.
##'   }
##'   \item{sha}{
##'     If the merge created a merge commit, the sha of the merge
##'     commit. NA if no merge commit created.
##'   }
##' }
