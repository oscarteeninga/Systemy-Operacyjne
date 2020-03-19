#ifndef SHARED_H
#define SHARED_H

#include <sys/time.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define UNIX_PATH_MAX 108
#define MAX_CLIENTS 10


typedef enum sock_msg_type_t {
    REGISTER,
    UNREGISTER,
    PING,
    PONG,
    OK,
    NAME_TAKEN,
    FULL,
    FAIL,
    WORK,
    WORK_DONE,
} sock_msg_type_t;

typedef struct sock_msg_t {
    uint8_t type;
    uint64_t size;
    uint64_t name_size;
    uint64_t id;
    void *content;
    char *name;
} sock_msg_t;

typedef struct client_t {
    int fd;
    char *name;
    struct sockaddr *addr;
    socklen_t addr_len;
    uint8_t working;
    uint8_t inactive;
} client_t;

int parse_pos_int(char*);

void show_errno(void);
void error_exit_errno(void);
void error_exit(char* msg);

#endif