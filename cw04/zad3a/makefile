CC = gcc
CFLAGS = -Wall

all: sender catcher

sender:
	$(CC) $(CFLAGS) sender.c -o sender

catcher:
	$(CC) $(CFLAGS) catcher.c -o catcher

clean:
	rm -r sender catcher
