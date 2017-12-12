context("git_sha")
test_that("git_sha returns the correct value", {
  file <- "test.txt"
  local.path <- "test"
  tmpdir <- tempfile(pattern = "git2r-git_sha")
  connection <- normalizePath(
    tmpdir,
    winslash = "/",
    mustWork = FALSE
  )
  df <- data.frame(x = 1, y = 1:10)

  expect_error(
    git_sha(
      file = file,
      connection = connection,
      commit.user = "me",
      commit.email = "me@me.com"
    ),
    "is not a git repo"
  )

  dir.create(paste(connection, local.path, sep = "/"), recursive = TRUE)
  repo <- init(connection)
  expect_error(
    git_sha(
      file = file,
      connection = connection,
      commit.user = "me",
      commit.email = "me@me.com"
    ),
    "no commits available"
  )

  connection <- git_connection(
    repo.path = connection,
    local.path = local.path,
    commit.user = "me",
    commit.email = "me@me.com"
  )
  hash <- write_delim_git(
    x = df,
    file = file,
    connection = connection
  )
  commit(connection, message = "test")
  expect_is(
    sha <- git_sha(
      file = file,
      connection = connection
    ),
    "data.frame"
  )
  expect_identical(
    sha,
    data.frame(
      SHA = hash,
      Path = local.path,
      File = file,
      stringsAsFactors = FALSE
    )
  )
  expect_true(
    all(file.remove(
      list.files(tmpdir, all.files = TRUE, recursive = TRUE, full.names = TRUE)
    ))
  )
  expect_true(
    all(file.remove(
      rev(list.dirs(tmpdir, recursive = TRUE, full.names = TRUE))
    ))
  )
})
