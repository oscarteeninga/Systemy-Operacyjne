CC = gcc
CFLAGS = -Wall -std=c99 -pthread -lrt

all: loader trucker

loader:
	$(CC) $(CFLAGS) loader.c tasma.c config.c -o loader

trucker:
	$(CC) $(CFLAGS) trucker.c tasma.c config.c -o trucker

clean:
	rm  -rf *.o trucker loader
