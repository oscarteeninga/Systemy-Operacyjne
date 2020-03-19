CC = gcc
CFLAGS = -Wall

all: copy monitor

monitor:
	$(CC) $(CFLAGS) -c monitor.c
	$(CC) $(CFLAGS) monitor.o -o monitor
	rm monitor.o

copy:
	cp test_cpy/* test

clean:
	rm test/* monitor