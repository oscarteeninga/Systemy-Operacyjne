CC = gcc
ARGS = -Wall

all: clean tester monitor copy

monitor:
	$(CC) $(ARGS) -c monitor.c
	$(CC) $(ARGS) monitor.o -o monitor
	rm monitor.o

tester:
	$(CC) $(ARGS) -c tester.c
	$(CC) $(ARGS) tester.o -o tester
	rm tester.o

clean:
	rm monitor tester test/*

copy:
	cp test_cpy/* test
