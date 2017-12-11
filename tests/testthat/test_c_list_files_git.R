context("list files for a git repository")
describe("list_files_git()", {
  files <- sort(c("test.txt", "0123456.txt", "test", "0123456"))
  local.path <- "test"
  connection <- tempfile(pattern = "git2r-")
  connection <- normalizePath(connection, winslash = "/", mustWork = FALSE)
  commit.user <- "me"
  commit.email <- "me@me.com"


  it("stops if connection is not a git repository", {
    expect_error(
      list_files_git(
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
  connection <- normalizePath(connection, winslash = "/", mustWork = FALSE)
  full.path <- paste(connection, local.path, sep = "/")
  full.path <- normalizePath(full.path, winslash = "/", mustWork = FALSE)
  it("stops if the local.path doesn't exist", {
    expect_error(
      list_files_git(
        local.path = local.path,
        connection = connection,
        commit.user = commit.user,
        commit.email = commit.email
      ),
      "is not a directory"
    )
  })

  dir.create(paste(connection, local.path, sep = "/"))
  file.create(paste(full.path, files, sep = "/"))
  it("list the files according to the pattern", {
    expect_that(
      list_files_git(
        local.path = local.path,
        pattern = "^[0123456789].*\\.txt$",
        connection = connection,
        commit.user = commit.user,
        commit.email = commit.email
      ),
      is_identical_to(files[grep("^[0123456789].*\\.txt$", files)])
    )
    expect_that(
      list_files_git(
        local.path = local.path,
        pattern = "\\.txt$",
        connection = connection,
        commit.user = commit.user,
        commit.email = commit.email
      ),
      is_identical_to(files[grep("\\.txt$", files)])
    )
    expect_that(
      list_files_git(
        local.path = local.path,
        connection = connection,
        commit.user = commit.user,
        commit.email = commit.email
      ),
      is_identical_to(files)
    )
    expect_that(
      list_files_git(
        local.path = local.path,
        pattern = ".exe",
        connection = connection,
        commit.user = commit.user,
        commit.email = commit.email
      ),
      is_identical_to(character(0))
    )
  })
})
