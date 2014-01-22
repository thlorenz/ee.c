CWD=$(shell pwd)

AR ?= ar
CC ?= gcc
PREFIX ?= /usr/local
SCANBUILD ?= scan-build

CFLAGS = -c -O3 -Wall -std=c99

LIST = deps/list

SRCS = $(LIST)/list.c $(LIST)/list_iterator.c $(LIST)/list_node.c src/ee.c
OBJS = $(SRCS:.c=.o)
INCS = -I $(LIST)/
CLIB = node_modules/.bin/clib

LIBEE = build/libee.a

all: clean $(LIBEE)

$(LIBEE): $(LIST) $(OBJS)
	@mkdir -p build
	$(AR) rcs $@ $(OBJS)

install: all
	cp -f $(LIBEE) $(PREFIX)/lib/libee.a
	cp -f src/ee.h $(PREFIX)/include/ee.h

uninstall:
	rm -f $(PREFIX)/lib/libee.a
	rm -f $(PREFIX)/include/ee.h

check:
	$(SCANBUILD) $(MAKE) test

test: $(LIST) test.o $(OBJS)
	@mkdir -p bin
	$(CC) $(OBJS) test.o -o bin/$@
	bin/$@

# clibs
$(CLIB):
	npm install

$(LIST): $(CLIB)
	$(CLIB) install clibs/list -o deps/

.SUFFIXES: .c .o
.c.o: 
	$(CC) $< $(CFLAGS) $(INCS) -c -o $@

clean-all: clean
	rm -f $(OBJS)

clean:
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`
	rm -rf bin src/*.o *.o

.PHONY: all check test clean clean-all install uninstall
