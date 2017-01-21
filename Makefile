#
# Author: Pablo Musa
# Creation Date: mar 27 2011
# Last Modification: feb 22 2015
# See Copyright Notice in COPYRIGHT
# 
# lmprof - A Memory Profiler for the Lua language
#
LUA_VERSION=5.3

CC = gcc
#CFLAGS = -g -O0 -Wall -ansi -pedantic -fPIC -shared
CFLAGS = -O2 -Wall -ansi -pedantic -fPIC -shared

#LUA_DIR = /usr/include/lua${LUA_VERSION}
LUA_DIR = /home/pmusa/Coding/lua/lua-${LUA_VERSION}.0/src
LUA_LIB = /usr/local/lib/lua/${LUA_VERSION}
LUA_CFLAGS = -I$(LUA_DIR)
LUA_LIBS = -llua${LUA_VERSION}

all: bin/lmprof.so

install: bin/lmprof.so
	mkdir -p $(LUA_LIB)/lmprof && cp src/reduce/*.lua $(LUA_LIB)/lmprof && cp bin/lmprof.so /usr/local/lib/lua/${LUA_VERSION}

bin/lmprof.so: src/lmprof.o src/lmprof_stack.o src/lmprof_hash.o src/lmprof_lstrace.o
	cd src && $(CC) lmprof_lstrace.o lmprof_stack.o lmprof_hash.o lmprof.o -o lmprof.so $(CFLAGS) $(LUACFLAGS) $(LUA_LIBS) && mv lmprof.so ../bin/

src/lmprof.o: src/lmprof.c
	cd src && $(CC) -c lmprof.c $(CFLAGS) $(LUA_CFLAGS)

src/lmprof_lstrace.o: src/lmprof_lstrace.c src/lmprof_lstrace.h
	cd src && $(CC) -c lmprof_lstrace.c $(CFLAGS) $(LUA_CFLAGS)

src/lmprof_hash.o: src/lmprof_hash.c src/lmprof_hash.h
	cd src && $(CC) -c lmprof_hash.c $(CFLAGS)

src/lmprof_stack.o: src/lmprof_stack.c src/lmprof_stack.h
	cd src && $(CC) -c lmprof_stack.c $(CFLAGS)

clean:
	rm -f src/*.o bin/lmprof.so

