context("remove_files_git")
test_that("remove_files_git returns the correct value", {
  file <- "test.txt"
  local.path <- "test"
  tmpdir <- tempfile(pattern = "git2r-remove_files_git")
  connection <- normalizePath(
    tmpdir,
    winslash = "/",
    mustWork = FALSE
  )
  df <- data.frame(x = 1, y = 1:10)
  pattern <- "txt$"

  expect_error(
    remove_files_git(
      connection = connection,
      commit.user = "me",
      commit.email = "me@me.com"
    ),
    "is not a git repo"
  )

  dir.create(paste(connection, local.path, sep = "/"), recursive = TRUE)
  repo <- init(connection)

  git_con <- git_connection(
    repo.path = connection,
    local.path = local.path,
    commit.user = "me",
    commit.email = "me@me.com"
  )
  hash <- write_delim_git(
    x = df,
    file = file,
    connection = git_con,
    commit.user = "me",
    commit.email = "me@me.com"
  )
  z <- commit(git_con, message = "test")
  expect_true(
    remove_files_git(
      connection = connection,
      local.path = local.path,
      commit.user = "me",
      commit.email = "me@me.com",
      pattern = "junk"
    )
  )
  expect_identical(
    list_files_git(git_con),
    file
  )
  expect_true(
    remove_files_git(
      connection = git_con,
      pattern = pattern
    )
  )
  expect_identical(
    list_files_git(git_con),
    character(0)
  )
  expect_identical(
    status(git_con@Repository)$staged$deleted,
    paste(local.path, file, sep = "/")
  )

  expect_identical(unlink(tmpdir, recursive = TRUE, force = TRUE), 0L)
})

test_that("returns an error when removing files fails", {
  skip_on_os(c("windows", "mac", "solaris"))

  file <- "test.txt"
  local.path <- "test"
  tmpdir <- tempfile(pattern = "git2r-remove_files_git_error")
  connection <- normalizePath(
    tmpdir,
    winslash = "/",
    mustWork = FALSE
  )
  df <- data.frame(x = 1, y = 1:10)
  pattern <- "txt$"
  dir.create(paste(connection, local.path, sep = "/"), recursive = TRUE)
  repo <- init(connection)

  git_con <- git_connection(
    repo.path = connection,
    local.path = local.path,
    commit.user = "me",
    commit.email = "me@me.com"
  )
  hash <- write_delim_git(
    x = df,
    file = file,
    connection = git_con
  )
  z <- commit(git_con, message = "test")

  Sys.chmod(
    paste(git_con@Repository@path, git_con@LocalPath, sep = "/"),
    mode = "0400"
  )
  expect_error(
    remove_files_git(
      connection = git_con,
      pattern = pattern
    ),
    "Error cleaning existing files"
  )

  Sys.chmod(
    paste(git_con@Repository@path, git_con@LocalPath, sep = "/"),
    mode = "0777"
  )
  expect_true(
    remove_files_git(
      connection = git_con,
      pattern = pattern
    ),
    "Error cleaning existing files"
  )
  expect_identical(unlink(tmpdir, recursive = TRUE, force = TRUE), 0L)
})
