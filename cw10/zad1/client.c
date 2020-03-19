#include "shared.h"

int sockfd;

void handle_sig(int signum) {
    if(signum == SIGINT) {
        send_message(sockfd, DEREGISTER, 0, NULL);
        printf("\nSIGINT\n");
    } else if(signum == SIGPIPE) {
        printf("\nServer is down");
    }
    close(sockfd);
    exit(0);
}

int count_words(char *text) {
    char *word = strtok(text, " \n");
    int count = 0;
    while(word != NULL) {
        word = strtok(NULL, " \n");
        count++;
    }
    return count;
}

void handle_received(ssize_t size, Type type, void *data) {
    switch(type) {
        case TEXT: {
            Result result;
            memcpy(result.name, data, MAX_NAME_LEN);
            memcpy(&(result.number), data + MAX_NAME_LEN, sizeof(int));
            printf("Counting words in file: %s, number: %d \n", result.name, result.number);
            char *dat = malloc(size);
            char *buffer = malloc(100 + 2 * size);
            memcpy(dat, data, size);
            result.words = count_words((char*) data + MAX_NAME_LEN + sizeof(int));
            send_message(sockfd, RESULT, sizeof(Result), &result);
            if(size < 1000)  {
                sprintf(buffer, "echo '%s' | awk '{for(x=1;$x;++x)print $x}' | sort | uniq -c", dat + MAX_NAME_LEN + sizeof(int));
                FILE *res = popen(buffer, "r");
                if (res == 0) { free(buffer); return; }
                int n = fread(buffer, 1, 99 + 2 * size, res);
                buffer[n] = '\0';
                send_message(sockfd, RESULT, n + 1, buffer);
                pclose(res);
            }
            else {
                send_message(sockfd, TEXT, 0, NULL);
            }
            puts("Work done...");
            free(buffer);
            free(dat);
            break;
        }
        case PING:
            send_message(sockfd, PING, 0, NULL);
            break;
        case NAME_TAKEN:
            break;
        case DEREGISTER:
            break;
        case RESULT:
            break;
        case NAME:
            break;
    }
}

int main(int argc, char **argv) {
    if(argc < 4) {
        fprintf(stderr, "Usage: ./client [NAME] [MODE] [ADDRESS]");
        exit(-1);
    }
    if(atoi(argv[2]) == 0) {
        const struct sockaddr_in addr = {
            .sin_family = AF_INET,
            .sin_port = htons(atoi(argv[3])),
            .sin_addr = {
                .s_addr = inet_addr("127.0.0.1")
            }
        };
        sockfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
        if(connect(sockfd, (const struct sockaddr*) &addr, sizeof(addr)) != 0) 
            error_exit("Connect failure");
    } else {
        struct sockaddr_un addr;
        addr.sun_family = AF_UNIX,
        memcpy(addr.sun_path, argv[3], strlen(argv[3]));
        sockfd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
        if(connect(sockfd, (const struct sockaddr*) &addr, sizeof(addr)) != 0) 
            error_exit("Connect failure");
    }
    signal(SIGINT, handle_sig);
    signal(SIGPIPE, handle_sig);
    if(sockfd == -1)
        error_exit("Failed to open the socket");
    char name[MAX_NAME_LEN] = {0};
    memcpy(name, argv[1], strlen(argv[1]));
    send_message(sockfd, NAME, MAX_NAME_LEN, name);
    Type type;
    void *data;
    receive_message(sockfd, &type, NULL);
    if(type == NAME_TAKEN) {
        printf("Name %s taken\n", argv[1]);
        return 1;
    }
    while(1) {
        ssize_t n = receive_message(sockfd, &type, &data);
        handle_received(n, type, data);
        free(data);
    }
}