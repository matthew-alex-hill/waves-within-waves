CC = gcc
CFLAGS = -Wall -g -fsanitize=undefined
LFLAGS = -l
LDLIBS = -lubsan
BUILD = www_lexer
OBJS = www_compiler.o lexsupport.o

all: $(BUILD)

clean:
	/bin/rm -f $(BUILD) *.o www_compiler.c core
	cd ../; /bin/rm -f wwwc core

www_lexer: $(OBJS)
	$(CC) -o ../wwwc $(OBJS) $(LDLIBS)

www_compiler.o: lexsupport.h
lexsupport.o: lexsupport.h

www_lexer.c: www_compiler.l
	lex $(LFLAGS) -o www_compiler.c www_compiler.l

