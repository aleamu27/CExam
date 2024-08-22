# Makefile template
INCLDIR = ./include
CC = gcc
CFLAGS = -O2 -Wall -pthread
CFLAGS += -I$(INCLDIR)
OBJDIR = obj
_DEPS = 
DEPS = $(patsubst %,$(INCLDIR)/%,$(_DEPS))
_OBJS = main.o
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCLDIR)/*~
