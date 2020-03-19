#define _XOPEN_SOURCE 500
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define TIME_FMT "%Y.%m.%d %H:%M:%S"

int show_date = 1;

static void SIG_TSTP(int);
static void SIG_INT(int);

int main(void) {
    char time_buffer[256];

    struct sigaction SIG_TSTP_ACTION;
    SIG_TSTP_ACTION.sa_handler = SIG_TSTP;
    sigemptyset(&SIG_TSTP_ACTION.sa_mask);
    SIG_TSTP_ACTION.sa_flags = 0;

    if (sigaction(SIGTSTP, &SIG_TSTP_ACTION, NULL)) {
        printf("Nie udało się określić obsługi sygnału SIG_TSTP.\n");
        exit(1);
    }

    if (signal(SIGINT, SIG_INT) == SIG_ERR) {
        printf("Nie udało się określić obsługi sygnału SIG_INT");
        exit(1);
    }

    while(1) {
        if (show_date) {
            time_t _time;
            struct tm *time_info;
            time(&_time);
            time_info = localtime(&_time);
            strftime(time_buffer, 255, TIME_FMT, time_info);
            puts(time_buffer);
        }
        usleep(1000000);
    }
    return 0;
}

static void SIG_TSTP(int useless) {
    if (show_date) {
        printf("Oczekuję na CTRL+Z - kontynuacja albo CTRL+c - zakonczenie programu.\n");
        show_date = 0;
    } else {
        show_date = 1;
    }
}

static void SIG_INT(int useless) {
    printf("Odebrano sygnał SIGINT!\n");

    if (signal(SIGINT, SIG_INT) == SIG_ERR) {
        printf("Nie udało się zmienić obsługi sygnału SIG_INT");
    }
    exit(1);
}