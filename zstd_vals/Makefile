RM_INCLUDE_DIR=../
RMUTIL_LIBDIR=../rmutil
ZSTD_INCLUDE_DIR=../zstd/lib
ZSTD_LIBDIR=../zstd/lib

# Compile flags for linux
SHOBJ_CFLAGS ?=  -fno-common -g -ggdb
SHOBJ_LDFLAGS ?= -shared -Bsymbolic
CFLAGS = -I$(RM_INCLUDE_DIR) -I$(ZSTD_INCLUDE_DIR) -Wall -g -fPIC -lc -lm -O2 -std=c++11
CC=g++
LD=g++
SRCS=TaskQueue.cpp TaskScheduler.cpp TaskWorker.cpp ZSETTask.cpp ZDICTSETTask.cpp zstd_vals.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: zstd_vals.so

zstd_vals.so: $(OBJS)
	$(LD) -o $@ $(OBJS) $(SHOBJ_LDFLAGS) $(LIBS) -L$(RMUTIL_LIBDIR) -lrmutil -lc -L$(ZSTD_LIBDIR) -lzstd

%.o: %.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

clean:
	rm -rf *.xo *.so *.o
