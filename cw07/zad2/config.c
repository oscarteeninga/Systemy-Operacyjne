#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "config.h"

int parse_pos_int(char *str) {
    int i = atoi(str);
    if (i <= 0) {
        die("arg");
        exit(-1);
    } else {
        return i;
    }
}

struct timeval curr_time() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time;
}

long int time_diff(struct timeval time1, struct timeval time2) {
    return (time2.tv_usec - time1.tv_usec + 1000000) % 1000000;
}

void print_time(struct timeval time) {
    printf("%ld%ld", time.tv_sec, time.tv_usec);
}

void print_current_time() {
    print_time(curr_time());
}

void die(char *msg) {
    perror(msg);
    exit(1);
}