CWD=$(shell pwd)

AR ?= ar
CC ?= gcc
PREFIX ?= /usr/local
SCANBUILD ?= scan-build

CFLAGS = -c -O3 -Wall -std=c99

LIST = deps/list

SRCS = $(LIST)/list.c $(LIST)/list_iterator.c $(LIST)/list_node.c src/ee.c
OBJS = $(SRCS:.c=.o)
INCS = -I$(LIST)/ -Isrc/
CLIB = node_modules/.bin/clib

TEST_SRCS = $(wildcard test/*.c)
TESTS = $(addprefix bin/,$(TEST_SRCS:.c=))

LIBEE = build/libee.a
EXAMPLE = example

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

test: $(TESTS) 
	set -e; for file in $^; do echo "\n\033[00;32m+++ $$file +++\033[00m\n" && ./$$file; done

bin/test/%: $(OBJS) test/%.o
	@mkdir -p bin/test
	$(CC) $(LDFLAGS) $^ -o $@ 
ifeq ($(uname),Darwin)
	dsymutil $@ 
endif

$(EXAMPLE): clean $(LIST) $(OBJS) example.o
	@mkdir -p bin
	$(CC) $(OBJS) example.o -o bin/$@
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

include valgrind.mk
