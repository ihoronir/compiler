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
	./test.js

clean:
	rm -f $(TARGET) $(OBJDIR)/*.o
	rm -f ./test/asm/*.s ./test/target/* ./test/log/*.log

.PHONY: test clean
