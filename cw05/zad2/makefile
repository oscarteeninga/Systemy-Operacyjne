CC = gcc
CFLAGS = -Wall

all: slave master

slave: 
	$(CC) $(CFLAGS) -c slave.c
	$(CC) $(CFLAGS) slave.o -o slave

master:
	$(CC) $(CFLAGS) -c master.c
	$(CC) $(CFLAGS) master.o -o master

clean:
	rm -r master slave slave.o master.o