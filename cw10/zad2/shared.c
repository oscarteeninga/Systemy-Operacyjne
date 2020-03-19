#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "shared.h"

int parse_pos_int(char* s) {
    int i = atoi(s);
    if (i <= 0) {
        fprintf(stderr, "%s should be a positive integer\n", s);
        exit(-1);
    } else {
        return i;
    }
}

void show_errno(void) {
	fputs(strerror(errno), stderr);
}

void error_exit_errno(void) {
	error_exit(strerror(errno));
}

void error_exit(char* msg) {
	assert(msg);
	fprintf(stderr, "%s\n", msg);
	exit(1);
}