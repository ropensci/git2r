context("write data.frame to git")
describe("write_delim_git()", {
  commit.user <- "me"
  commit.email <- "me@me.com"
  x <- data.frame(0)
  x1 <- data.frame(1)
  file <- "test.txt"
  local.path <- "test/subdir"
  connection <- normalizePath(
    tempfile(pattern = "git2r-"),
    winslash = "/",
    mustWork = FALSE
  )

  it("stops if connection is not a git repository", {
    expect_error(
      write_delim_git(
        x = x,
        file = file,
        local.path = local.path,
        connection = connection,
        commit.user = commit.user,
        commit.email = commit.email
      ),
      "is not a git repo"
    )
  })

  dir.create(connection)
  repo <- git2r::init(connection)
  git2r::config(repo, user.name = "me", user.email = "me@me.com")
  it("stops if the path doesn't exist", {
    expect_error(
      write_delim_git(
        x = x,
        file = file,
        local.path = local.path,
        connection = connection,
        commit.user = commit.user,
        commit.email = commit.email
      ),
      "is not a directory"
    )
  })
  full.path <- paste(connection, local.path, sep = "/")
  dir.create(full.path, recursive = TRUE)
  it("stops if x is not a data.frame", {
    expect_error(
      write_delim_git(
        x = matrix(0),
        file = file,
        local.path = local.path,
        connection = connection,
        commit.user = commit.user,
        commit.email = commit.email
      ),
      "x does not inherit from class data.frame"
    )
  })

  full.file.path <- paste(connection, local.path, file, sep = "/")
  it("returns the sha1 of the file", {
    expect_that(
      write_delim_git(
        x = x,
        file = file,
        local.path = local.path,
        connection = connection,
        commit.user = commit.user,
        commit.email = commit.email
      ),
      is_identical_to(hashfile(full.file.path))
    )
  })
  it("can handle tbl_df", {
    expect_that(
      write_delim_git(
        x = dplyr::as.tbl(x),
        file = file,
        local.path = local.path,
        connection = connection,
        commit.user = commit.user,
        commit.email = commit.email
      ),
      is_identical_to(hashfile(full.file.path))
    )
  })

  it("stages the file", {
    expect_that(
      status(repo)$staged$new,
      is_identical_to(paste(local.path, file, sep = "/"))
    )
    junk <- commit(repo, "a")
    write_delim_git(
      x = x1,
      file = file,
      local.path = local.path,
      connection = connection,
      commit.user = commit.user,
      commit.email = commit.email
    )
    expect_that(
      status(repo)$staged$modified,
      is_identical_to(paste(local.path, file, sep = "/"))
    )
  })
})
