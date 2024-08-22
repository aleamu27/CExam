CC = gcc
CFLAGS = -I.
LIBS = -lm

_DEPS = main.h
DEPS = $(_DEPS)

OBJS = main.o konte_h23_oppgave2_fib.o konte_h23_oppgave2_abun.o konte_h23_oppgave2_def.o konte_h23_oppgave2_kvad.o konte_h23_oppgave2_odd.o konte_h23_oppgave2_perf.o konte_h23_oppgave2_prim.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f *.o *~ core *~ main
