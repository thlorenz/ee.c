CWD=$(shell pwd)

SRC_PATH=$(CWD)
INCLUDE_PATH=$(CWD)
DEPS_PATH=$(CWD)/deps

LOGH_PATH=$(DEPS_PATH)/log.h

CC=cc
CFLAGS=-c -O2 -Wall -std=c99
INCLUDES=-I/$(LOGH_PATH)/ -I/$(INCLUDE_PATH)/
LIBS=

uname_S=$(shell uname -s)

ifeq (Darwin, $(uname_S))
CFLAGS+=
#-framework CoreServices
endif

ifeq (Linux, $(uname_S))
LIBS=-lrt -ldl -lm -pthread
endif

CENT_SRC=$(SRC_PATH)/cent.c
CENT_OBJECTS=$(CENT_SRC:.c=.o)
CENT=cent

all: clean $(CENT)

run: all
	@echo "\n\033[1;33m>>>\033[0m"
	./$(CENT)
	@echo "\n\033[1;33m<<<\033[0m\n"
	make clean

$(CENT): $(CENT_OBJECTS)
	$(CC) $(LIBS) $^ -o $@

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`
	rm -f $(CENT) $(CENT_OBJECTS) 

.PHONY: all clean
