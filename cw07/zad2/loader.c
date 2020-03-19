#define _POSIX_C_SOURCE 200122L
#define _XOPEN_SOURCE 1

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "tasma.h"
#include "config.h"

void register_handlers();
void exit_handler();
void int_handler(int);

int main(int argc, char *argv[]) {
    if (argc < 2) die("Wprowadz ilosc ladowniczych oraz ich czas zycia\n");
    int child_count = parse_pos_int(argv[1]);
    for (int i = 0; i < child_count; i++) {
        if (fork() == 0) {
            int m = parse_pos_int(argv[2+i]);
            belt *b = get_belt();
            register_handlers();
            package pkg = create_package(m);
            if (argc > 2 + i + child_count) {
                for (int j = 0; j < parse_pos_int(argv[2 + i + child_count]); j++) {
                    put_package(b, pkg);
                }
            } else {
                while (1) {
                    put_package(b, pkg);
                }
            }
        }
    }

    for (int i = 0; i < child_count; i++) wait(NULL);

    return 0;
}

void register_handlers() {
    struct sigaction action;
    action.sa_handler = int_handler;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGINT, &action, NULL) == -1) die("sigaction");
}

void int_handler(int useless) {}
