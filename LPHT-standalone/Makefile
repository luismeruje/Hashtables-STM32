CC=gcc
CFLAGS=-Wall
IFLAGS=-I inc
SRCDIR=src

all: CFLAGS += -O3
all: LPHT-standalone-test
debug: CFLAGS += -g -O0
debug: LPHT-standalone-test

LPHT-standalone-test: $(SRCDIR)/benchmark_lpht.o $(SRCDIR)/lpht.o $(SRCDIR)/crc32.o
	$(CC) $^ -o $@

$(SRCDIR)/crc32.o: $(SRCDIR)/crc32.c
	$(CC) $(CFLAGS) $(IFLAGS) -c $^ -o $@

$(SRCDIR)/lpht.o: $(SRCDIR)/lpht.c
	$(CC) $(CFLAGS) $(IFLAGS) -c $^ -o $@

$(SRCDIR)/benchmark_lpht.o: $(SRCDIR)/benchmark_lpht.c
	$(CC) $(CFLAGS) $(IFLAGS) -c $^ -o $@

clean:
	rm src/*.o
	rm LPHT-standalone-test

