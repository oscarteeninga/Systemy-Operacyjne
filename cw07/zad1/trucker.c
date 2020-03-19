#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "tasma.h"

void register_handlers();
void exit_handler();
void int_handler(int);

belt *b;

int main(int argc, char *argv[]) {
    if (argc != 4) die("Wprowadz X, K i M");
    int x = parse_pos_int(argv[1]);
    int k = parse_pos_int(argv[2]);
    int m = parse_pos_int(argv[3]);

    b = create_belt(k, m);

    register_handlers();

    truck t;
    t.capacity = x;
    t.max_capacity = x;

    while(1) {
        get_package(b, &t);
    }

    return 0;
}

void register_handlers() {
    struct sigaction action;
    action.sa_handler = int_handler;
    sigemptyset(&action.sa_mask);

    if (sigaction(SIGINT, &action, NULL) == -1) die("sigaction");

    atexit(exit_handler);
}

void exit_handler() {
    if (b) delete_belt(b);
}

void int_handler(int useless) {
    b->trucker_done = 1;
    exit(0);
}