CC = gcc
CFLAGS = -Wall
LFLAGS = -l
LDLIBS =
BUILD = lex_test
OBJS = lex_test.o www_lexer.o lexsupport.o

all: $(BUILD)

clean:
	/bin/rm -f $(BUILD) *.o www_lexer.c core

lex_test: $(OBJS)
	$(CC) -o lex_test $(OBJS) $(LDLIBS)

www_lexer.o: lexsupport.h
lexsupport.o: lexsupport.h
lex_test.o: lexsupport.h

www_lexer.c: www_lexer.l
	lex $(LFLAGS) -o www_lexer.c www_lexer.l
