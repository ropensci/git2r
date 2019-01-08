# The C compiler to use.
CC=gcc

# The location of the libgit2 include files.
LIBGIT2_INCLUDE=/usr/local/Cellar/libgit2/0.27.7/include

# The location of the libgit2 library files.
LIBGIT2_LIB=/usr/local/Cellar/libgit2/0.27.7/lib

# RULES
# -----
all: git_log

common.o: common.c
	$(CC) -c $^ -I$(LIBGIT2_INCLUDE)

log.o: log.c
	$(CC) -c $^ -I$(LIBGIT2_INCLUDE)

git_log: common.o log.o
	$(CC) -o git_log $^ -L$(LIBGIT2_LIB) -lgit2

clean:
	rm -f git_log common.o log.o


