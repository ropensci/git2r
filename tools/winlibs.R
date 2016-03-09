# We need https
if(getRversion() < "3.3.0") setInternet2()

# Download libz
if(!file.exists("../windows/libz-1.2.8/include/zlib.h")){
  download.file("https://github.com/rwinlib/libz/archive/v1.2.8.zip", "lib.zip", quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  unzip("lib.zip", exdir = "../windows")
  unlink("lib.zip")
}

# Download OpenSSL
if(!file.exists("../windows/openssl-1.0.2d/include/openssl/ssl.h")){
  download.file("https://github.com/rwinlib/openssl/archive/v1.0.2d.zip", "lib.zip", quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  unzip("lib.zip", exdir = "../windows")
  unlink("lib.zip")
}

# Download libssh2
if(!file.exists("../windows/libssh2-1.6.0/include/libssh2.h")){
  download.file("https://github.com/rwinlib/libssh2/archive/v1.6.0.zip", "lib.zip", quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  unzip("lib.zip", exdir = "../windows")
  unlink("lib.zip")
}
