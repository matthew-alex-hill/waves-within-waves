CC = gcc
CFLAGS = -Wall -Wextra -g -O4 -D_DEFAULT_SOURCE -std=c99 -pedantic
LIB = libwave_utils.a
LIBOBJS = wave_generation.o synth_source.o midi_management.o
BUILD = $(LIB)

all: $(BUILD)

$(LIB): $(LIBOBJS)
	ar rcs $(LIB) $(LIBOBJS)

wave_generation.o: wave_generation.h

synth_source.o: synth_source.h

midi_management.o: midi_management.h

clean:
	/bin/rm -f $(BUILD) *.o core
