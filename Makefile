CFLAGS	:= -std=c11 -g -O0 -static -Wall
LDFLAGS := -static

CC := gcc

INCLUDE := -I./include -I/usr/include

SRCDIR := ./src
OBJDIR := ./obj

SRCS := $(wildcard ./$(SRCDIR)/*.c)
OBJS := $(addprefix ./$(OBJDIR)/, $(notdir $(SRCS:.c=.o)))

TARGET := ./target/compiler

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c 
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

test: $(TARGET)
	./test.js ./test_working

test_todo: $(TARGET)
	./test.js ./test_todo

clean:
	rm -f $(TARGET) $(OBJDIR)/*.o
	rm -f ./test*/asm/*.s ./test*/target/* ./test*/log/*.log
	make clean -C ./test/lib

.PHONY: test clean
