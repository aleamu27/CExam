# Makefile for server on Debian 12

CC = gcc
CFLAGS = -Wall -g -O2

OBJDIR = obj

_DEPS = konte_h24_oppgave3.h
DEPS = $(_DEPS)

_OBJS = server.o konte_h24_oppgave3.o
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

# Create obj directory if it doesn't exist
$(shell mkdir -p $(OBJDIR))

all: server

server: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean all

clean:
	rm -rf $(OBJDIR) *~ core server

# Debugging target
debug:
	@echo "DEPS: $(DEPS)"
	@echo "OBJS: $(OBJS)"
	@ls -R
