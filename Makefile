CFLAGS=-std=c11 -g -static

compiler: compiler.c

test: compiler
	./test.sh

clean:
	rm -f compiler *.o *~ tmp*

.PHONY: test clean

