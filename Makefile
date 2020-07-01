CC = gcc
CFLAGS = -Wall -Wextra -g -D_DEFAULT_SOURCE -std=c99 -pedantic
LIB = libwave_utils.a
LIBOBJS = wave_generation.o
BUILD = $(LIB)

all: $(BUILD)

$(LIB): $(LIBOBJS)
	ar rcs $(LIB) $(LIBOBJS)

wave_generation.o: wave_generation.h

clean:
	/bin/rm -f $(BUILD) *.o core
