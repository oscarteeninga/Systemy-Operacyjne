CC = gcc
CFLAGS = -Wall -pthread

all: server client	


server: server.c shared.h shared.c
	$(CC) $(CFLAGS) server.c shared.c -o server

client: client.c shared.h shared.c
	$(CC) $(CFLAGS) client.c shared.c -o client

clean:	
	rm -rf *.o server client