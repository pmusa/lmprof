#
# Author: Pablo Musa
# Creation Date: may 19 2014
# Last Modification: ago 15 2014
# See Copyright Notice in COPYRIGHT
#
LIB_NAME=lmprof

CC = gcc
CFLAGS = -g -Wall -ansi -pedantic -fPIC -shared
CFLAGS = -g -Wall -pedantic -fPIC -shared

LUA_DIR = /usr/include/lua5.2
LUA_CFLAGS = -I$(LUA_DIR)
LUA_LIBS = -llua5.2

all: $(LIB_NAME).so

lmprof.so: $(LIB_NAME).o
	cd src && $(CC) lmprof_stack.o lmprof_hash.o $(LIB_NAME).o -o $(LIB_NAME).so $(CFLAGS) $(LUA_LIBS) && mv $(LIB_NAME).so ../

lmprof.o: lmprof_stack.o lmprof_hash.o
	cd src && $(CC) -c $(LIB_NAME).c $(CFLAGS) $(LUA_CFLAGS)

lmprof_hash.o:
	cd src && $(CC) -c lmprof_hash.c $(CFLAGS)

lmprof_stack.o:
	cd src && $(CC) -c lmprof_stack.c $(CFLAGS)

clean:
	rm src/*.o

test:
	./run.sh

