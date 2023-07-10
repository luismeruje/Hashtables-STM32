CC=gcc
CFLAGS=-Wall
IFLAGS=-I inc
SRCDIR=src

all: CFLAGS += -O3
all: CLHT-standalone-test
debug: CFLAGS += -g -O0
debug: CLHT-standalone-test

CLHT-standalone-test: $(SRCDIR)/benchmark_clht.o $(SRCDIR)/clht_lb.o
	$(CC) $^ -o $@

$(SRCDIR)/benchmark_clht.o: $(SRCDIR)/benchmark_clht.c
	$(CC) $(CFLAGS) $(IFLAGS) -c $^ -o $@

$(SRCDIR)/clht_lb.o: $(SRCDIR)/clht_lb.c
	$(CC) $(CFLAGS) $(IFLAGS) -c $^ -o $@


clean:
	rm src/*.o
	rm CLHT-standalone-test

