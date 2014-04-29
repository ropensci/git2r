o_files <- function(path, exclude = NULL) {
    files <- sub("c$", "o",
                 sub("src/", "",
                     list.files(path, pattern = "[.]c$", full.names = TRUE)))

    if(!is.null(exclude))
        files <- files[files != exclude]
    files
}

build_objects <- function(files, Makevars) {
    lapply(names(files), function(obj) {
        cat("OBJECTS.", obj, " =", sep="", file = Makevars)
        len <- length(files[[obj]])
        for(i in seq_len(len)) {
            prefix <- ifelse(all(i > 1, (i %% 3) == 1), "    ", " ")
            postfix <- ifelse(all(i > 1, (i %% 3) == 0), " \\\n", "")
            cat(prefix, files[[obj]][i], postfix, sep="", file = Makevars)
        }
        cat("\n\n", file = Makevars)
    })

    cat("OBJECTS =", file = Makevars)
    len <- length(names(files))
    for(i in seq_len(len)) {
        prefix <- ifelse(all(i > 1, (i %% 3) == 1), "    ", " ")
        postfix <- ifelse(all(i > 1, (i %% 3) == 0), " \\\n", "")
        cat(prefix, "$(OBJECTS.", names(files)[i], ")", postfix, sep="", file = Makevars)
    }
    cat("\n", file = Makevars)

    invisible(NULL)
}

build_Makevars.in <- function() {
    Makevars <- file("src/Makevars.in", "w")
    on.exit(close(Makevars))

    files <- list(libgit2            = o_files("src/libgit2"),
                  libgit2.hash       = o_files("src/libgit2/hash", "libgit2/hash/hash_win32.o"),
                  libgit2.transports = o_files("src/libgit2/transports"),
                  libgit2.unix       = o_files("src/libgit2/unix"),
                  libgit2.xdiff      = o_files("src/libgit2/xdiff"),
                  http_parser        = o_files("src/http-parser"),
                  root               = o_files("src"))

    cat("PKG_CPPFLAGS = @CPPFLAGS@\n", file = Makevars)
    cat("PKG_LIBS = @LIBS@\n", file = Makevars)
    cat("PKG_CFLAGS = -Ilibgit2 -Ilibgit2/include -Ihttp-parser -DGIT_SSL\n", file = Makevars)
    cat("\n", file = Makevars)

    build_objects(files, Makevars)
}

build_Makevars.win <- function() {
    Makevars <- file("src/Makevars.win", "w")
    on.exit(close(Makevars))

    files <- list(libgit2            = o_files("src/libgit2"),
                  libgit2.hash       = o_files("src/libgit2/hash"),
                  libgit2.transports = o_files("src/libgit2/transports"),
                  libgit2.xdiff      = o_files("src/libgit2/xdiff"),
                  libgit2.win32      = o_files("src/libgit2/xdiff"),
                  http_parser        = o_files("src/http-parser"),
                  regex              = o_files("src/regex"),
                  root               = o_files("src"))

    cat("PKG_LIBS = $(ZLIB_LIBS) -lws2_32\n", file = Makevars)
    cat("PKG_CFLAGS = -I. -Ilibgit2 -Ilibgit2/include -Ihttp-parser -Iwin32 -Iregex -DWIN32 -D_WIN32_WINNT=0x0501 -D__USE_MINGW_ANSI_STDIO=1\n", file=Makevars)
    cat("\n", file = Makevars)

    build_objects(files, Makevars)
}

build_Makevars.in()
build_Makevars.win()
