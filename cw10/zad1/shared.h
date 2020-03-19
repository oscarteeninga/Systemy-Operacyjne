#ifndef SHARED_H
#define SHARED_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define MAX_CLIENTS 20
#define MAX_NAME_LEN 40

typedef enum {
    TEXT,
    RESULT,
    NAME,
    NAME_TAKEN,
    DEREGISTER,
    PING
} Type;

typedef struct {
    char name[MAX_NAME_LEN];
    int number;
    int words;
} Result;

void error_exit(char *error);
void send_message(int sockfd, Type type, ssize_t size, void *data);
ssize_t receive_message(int sockfd, Type *type, void **data);

#endif