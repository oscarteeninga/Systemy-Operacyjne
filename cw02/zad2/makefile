CC = gcc
CFLAGS = -Wall
TEST_DIR = test_dir

all: compile test

compile:
	$(CC) $(CFLAGS) -c main.c
	$(CC) $(CFLAGS) main.o -o main

test:
	echo " ======= opendir, readdir and stat ======= "
	./main $(TEST_DIR) ">" "11.03.2019 21:00:05" 1
	echo "\n\n"

	echo " ================= nftw ================== "
	./main $(TEST_DIR) ">" "11.03.2019 21:00:05" 2
	echo " \n\n"

clean:
	rm main main.o