CFLAGS=-std=c11 -g -static -Wall
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

compiler: $(OBJS)
	$(CC) -o compiler $(OBJS) $(LDFLAGS)

$(OBJS): compiler.h

test: compiler
	./test.sh

clean:
	rm -f compiler *.o *~ tmp*

.PHONY: test clean
