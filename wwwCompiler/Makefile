CC = gcc
CFLAGS = -Wall -g -fsanitize=undefined
LFLAGS = -l
LDLIBS = -lubsan
BUILD = www_lexer
OBJS = www_lexer.o lexsupport.o

all: $(BUILD)

clean:
	/bin/rm -f $(BUILD) *.o www_lexer.c core

lex_test: $(OBJS)
	$(CC) -o lex_test $(OBJS) $(LDLIBS)

www_compile: $(OBJS)
	$(CC) -o www_compile $(OBJS) $(LDLIBS)

www_lexer: $(OBJS)
	$(CC) -o www_lexer $(OBJS) $(LDLIBS)

www_lexer.o: lexsupport.h
lexsupport.o: lexsupport.h
lex_test.o: lexsupport.h
www_compile.o: lexsupport.h
www_lexer.o: lexsupport.h

www_lexer.c: www_lexer.l
	lex $(LFLAGS) -o www_lexer.c www_lexer.l

