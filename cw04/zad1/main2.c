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

pid_t child_pid;

static void SIG_TSTP(int);
static void SIG_INT(int);

void spawn_child();

int main() {
    struct sigaction SIG_TSTP_ACTION;
    SIG_TSTP_ACTION.sa_handler = SIG_TSTP;
    sigemptyset(&SIG_TSTP_ACTION.sa_mask);
    SIG_TSTP_ACTION.sa_flags = 0;

    if (sigaction(SIGTSTP, &SIG_TSTP_ACTION, NULL)) {
        printf("Nie udało zmienić sie obsługi sygnału SIG_TSTP.\n");
        exit(1);
    }

    if (signal(SIGINT, SIG_INT) == SIG_ERR) {
        printf("Nie udało się zmienić obsługi sygnału SIG_INT");
        exit(1);
    }

    spawn_child();

    while(1) pause();

    return 0;
}

static void SIG_TSTP(int uneless) {
    if (show_date) {
        printf("Oczekuję na CTRL+Z - kontynuacja albo CTRL+c - zakonczenie programu.\n");
        show_date = 0;
        kill(child_pid, SIGINT);
    } else {
        spawn_child();
        show_date = 1;
    }
}

static void SIG_INT(int useless) {
    printf("Odebrano sygnał SIGINT!\n");
    if (show_date) {
        kill(child_pid, SIGINT);
    }
    exit(1);
}

void spawn_child() {
    pid_t child = vfork();
    if (child == 0) {
        execl("./date", "./date", NULL);
        printf("Nie udało się uruchomić skrypt!\n");
        exit(1);
    } else {
        child_pid = child;
    }
}