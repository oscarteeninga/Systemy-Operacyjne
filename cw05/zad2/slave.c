#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>


#define LINE_MAXI 256

void stop(char*);

int main(int argc, char *argv[]) {
    if (argc != 3) stop("Not correct argument set");

    int count = atoi(argv[2]);

    char date_res[LINE_MAXI];
    char pipe_string[LINE_MAXI+20];

    printf("%d\n", (int) getpid());

    int pipe = open(argv[1], O_WRONLY);
    if (pipe < 0) stop("Could not create pipe");

    srand(time(NULL));

    for (int i = 0; i < count; i++) {
        FILE *date = popen("date", "r");
        fgets(date_res, LINE_MAXI, date);

        if (sprintf(pipe_string, "PID: %d DATE: %s", (int) getpid(), date_res) < 0) stop("Could not fill pipe_string");
        if (write(pipe, pipe_string, strlen(pipe_string)) < 0) stop("Could not write line to pipe");

        sleep(rand() % 4 + 1);
    }
    close(pipe);
    return 0;
}

void stop(char *message) {
    printf("%s.\n", message);
    exit(1);
}
