CC      = gcc
CFLAGS  = -Wall -Wextra -Ilib -fsanitize=undefined -O4 -g -D_DEFAULT_SOURCE -std=c99 -pedantic
LDLIBS  = -Lutils -lwave_utils -lm -lubsan -L../../libraries/portaudio/lib -lportaudio
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
