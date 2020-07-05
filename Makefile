CC      = gcc
CFLAGS  = -Wall -Wextra -Ilib -g -D_DEFAULT_SOURCE -std=c99 -pedantic
LDLIBS  = -Lutils -lwave_utils
BUILD   = libs synth

.SUFFIXES: .c .o

.PHONY: all clean

all: $(BUILD)

synth: synth.o

libs:
	cd utils; make

clean:
	rm -f $(wildcard *.o)
	rm -f synth
	cd utils; make clean
