git_log_file <- function(file) {
  file <- workflowr:::absolute(file)
  r <- git2r::repository(path = dirname(file))
  blobs <- git2r::odb_blobs(r)
  blobs$fname <- ifelse(blobs$path == "", blobs$name,
                        file.path(blobs$path, blobs$name))
  blobs$fname_abs <- file.path(workflowr:::git2r_workdir(r), blobs$fname)
  blobs_file <- blobs[blobs$fname_abs == file,]

  # Ignore blobs that don't map to commits (caused by `git commit --amend`)
  git_log <- git2r::commits(r)
  git_log_sha <-
    vapply(git_log, function(x) workflowr:::git2r_slot(x, "sha"),character(1))
  blobs_file <- blobs_file[blobs_file$commit %in% git_log_sha, ]
  return(rev(blobs_file[, "commit"]))
}
