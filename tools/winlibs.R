# We need https
if(getRversion() < "3.3.0") setInternet2()
LIBSSH2 <- commandArgs(TRUE)

# Downloads libssh2 + dependencies
if(!file.exists(sprintf("../windows/libssh2-%sinclude/libssh2.h", LIBSSH2))){
  download.file(sprintf("https://github.com/rwinlib/libssh2/archive/v%s.zip", LIBSSH2), "lib.zip", quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  unzip("lib.zip", exdir = "../windows")
  unlink("lib.zip")
}
