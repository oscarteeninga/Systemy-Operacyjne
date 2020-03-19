#include "shared.h"

void error_exit(char *error) {
    char message[strlen(error) + 7];
    sprintf(message, "Error - %s", error);
    perror(message);
    exit(errno);
}

void send_message(int sockfd, Type type, ssize_t size, void *data) {
    write(sockfd, &type, sizeof(type));
    write(sockfd, &size, sizeof(size));
    if(size > 0) {
        write(sockfd, data, size);
    }
}

ssize_t receive_message(int sockfd, Type *type, void **data) {
    ssize_t size = 0;
    if(read(sockfd, type, sizeof(Type)) < 1) return -1;
    if(read(sockfd, &size, sizeof(size)) < 1) return -1;
    if(data != NULL) {
        void *pointer = malloc(size);
        for(int i = 0; i < size; i++) {
            if(read(sockfd, pointer + i, 1) < 1) return -1;
        }
        *data = pointer;
    }
    return size;
}