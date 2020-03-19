CC = gcc
CFLAGS = -Wall

all: main1 main2

main1:
	$(CC) $(FLAGS) -c main1.c
	$(CC) $(FLAGS) main1.o -o main1
	rm main1.o

main2:
	$(CC) $(FLAGS) -c main2.c
	$(CC) $(FLAGS) main2.o -o main2
	rm main2.o

clean:
	rm main1 main2
