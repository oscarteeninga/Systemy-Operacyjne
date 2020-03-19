#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define TIME_FMT "%Y.%m.%d %H:%M:%S"


int main(int argc, char *argv[]) {
    srand(time(NULL));
    if (argc != 5) {
        printf("Not enought arguments");
        exit(1);
    }

    char *file_name = argv[1];
    int pmin = atoi(argv[2]);
    int pmax = atoi(argv[3]);
    int bytes = atoi(argv[4]);

    char *str = calloc(bytes+1, sizeof(char));
    assert(str);

    for(int i = 0; i < bytes; i++) str[i] = 'a';
    str[bytes] = '\0';

    char *date = calloc(30, sizeof(char));
    assert(date);

    while(1) {
        int wait = rand() % (abs(pmax-pmin) + 1) + pmin;
        sleep(wait);

        FILE *file = fopen(file_name, "a");

        time_t t = time(NULL);

        strftime(date, 30, TIME_FMT, gmtime(&t));

        fprintf(file, "\n%d %s %s", wait, date, str);

        fclose(file);
    }
}