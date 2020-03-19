CFLAGS = -Wall
CC = gcc

main:
	$(CC) $(CFLAGS) zad2.c -o zad2

clean:
	rm -f ./zad2.o ./zad2

test:  zad2
	@echo -e "\n\nTESTING\n\n"
	./zad2 good
	@echo -e "=========================================================="
	./zad2 bad

all: zad2 test
