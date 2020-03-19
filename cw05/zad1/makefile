CC = gcc
CFLAGS = -Wall -std=gnu99

all: clean main

main:
	$(CC) $(CFLAGS) -c main.c
	$(CC) $(CFLAGS) main.o -o main

clean:
	rm main main.o