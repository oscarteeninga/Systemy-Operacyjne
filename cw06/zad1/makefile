CC = gcc
CFLAGS = -Wall -std=gnu99

all: server client

server:
	$(CC) $(CFLAGS) server.c config.c -o server

client:
	$(CC) $(CFLAGS) client.c config.c -o client

clean:
	rm  -rf *.o server client