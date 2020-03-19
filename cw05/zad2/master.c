#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>


#define LINE_MAXI 256

void stop(char*);

int main(int argc, char *argv[]) {
    srand(time(NULL));
    if (argc != 2) stop("Not correct argument set");

    char line[LINE_MAXI];

    if (mkfifo(argv[1], S_IWUSR | S_IRUSR) < 0) stop("Could not create FIFO file");

    FILE* pipe = fopen(argv[1], "r");
    if (!pipe) stop("Could not open FIFO file");

    while (fgets(line, LINE_MAXI, pipe) != NULL) {
        if(write(1, line, strlen(line)) < 0) stop("Could not write line");
    }
    fclose(pipe);
    return 0;    
}

void stop(char *message) {
    printf("%s.\n", message);
    exit(1);
}
