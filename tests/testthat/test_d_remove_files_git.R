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
