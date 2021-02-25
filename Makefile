CC      = gcc
CFLAGS  = -Wall -Wextra -Ilib -fsanitize=undefined -O4 -g -D_DEFAULT_SOURCE -std=c99 -pedantic
LDLIBS  = -Lutils -lwave_utils -lm -lubsan -L/home/matthew/Documents/C\ Libraries/pa_stable_v190600_20161030/portaudio/lib -lportaudio -L../../C\ Libraries/portmedia-code-r234/portmidi/trunk/Release -lportmidi
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
