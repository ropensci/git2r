context("read data.frame from git")
describe("read_delim_git()", {
  file <- "test.txt"
  local.path <- "test"
  tmpdir <- tempfile(pattern = "git2r-read_delim_git")
  connection <- normalizePath(
    tmpdir,
    winslash = "/",
    mustWork = FALSE
  )
  df <- data.frame(x = 1, y = 1:10)


  it("stops if connection is not a git repository", {
    expect_error(
      read_delim_git(
        file = file,
        local.path = local.path,
        connection = connection,
        commit.user = "me",
        commit.email = "me@me.com"
      ),
      "is not a git repo"
    )
  })

  dir.create(paste(connection, local.path, sep = "/"), recursive = TRUE)
  repo <- init(connection)

  it("returns FALSE when the file doesn't exists", {
    expect_error(
      read_delim_git(
        file = file,
        local.path = local.path,
        connection = connection,
        commit.user = "me",
        commit.email = "me@me.com"
      ),
      paste0(repo@path, "/", local.path, "/", file)
    )
  })
  write_delim_git(
    x = df,
    file = file,
    local.path = local.path,
    connection = connection,
    commit.user = "me",
    commit.email = "me@me.com"
  )
  it("read the tab-delimited file", {
    expect_that(
      read_delim_git(
        file = file,
        local.path = local.path,
        connection = connection,
        commit.user = "me",
        commit.email = "me@me.com"
      ),
      is_equivalent_to(df)
    )
  })
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
