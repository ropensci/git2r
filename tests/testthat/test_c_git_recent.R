context("git_recent")
test_that("git_recent works", {
  file <- "test.txt"
  local.path <- "test"
  tmpdir <- tempfile(pattern = "git2r-git_recent")
  connection <- normalizePath(
    tmpdir,
    winslash = "/",
    mustWork = FALSE
  )
  df <- data.frame(x = 1, y = 1:10)

  dir.create(paste(connection, local.path, sep = "/"), recursive = TRUE)
  repo <- init(connection)

  expect_error(
    git_recent(
      file = file,
      local.path = local.path,
      connection = connection,
      commit.user = "me",
      commit.email = "me@me.com"
    ),
    "no commits in current branch"
  )

  connection <- git_connection(
    repo.path = connection,
    local.path = local.path,
    commit.user = "me",
    commit.email = "me@me.com"
  )
  write_delim_git(
    x = df,
    file = file,
    connection = connection
  )
  z <- commit(connection, "test")
  expect_is(
    x <- git_recent(file = file, connection = connection),
    "list"
  )

  expect_identical(
    x,
    list(
      Commit = z@sha,
      Author = sprintf("%s <%s>", z@author@name, z@author@email),
      Date = as.POSIXct(as(z@author@when, "character"))
    )
  )

  expect_identical(unlink(tmpdir, recursive = TRUE, force = TRUE), 0L)
})
