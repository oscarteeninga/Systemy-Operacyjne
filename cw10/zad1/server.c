#include "shared.h"

int clients[MAX_CLIENTS];
char client_names[MAX_CLIENTS][MAX_NAME_LEN];
int client_states[MAX_CLIENTS];
int client_working[MAX_CLIENTS];
int pings[MAX_CLIENTS];
int sockfd, sockfd2;
int last_client = 0;
pthread_mutex_t clients_mutex;
char socket_name[40];

void handle_int(int signum) {
    printf("\nSIGINT\n");
    exit(0);
}

void handle_exit() {
    for(int i = 0; i < last_client; i++) {
        shutdown(clients[i], SHUT_RDWR);
    }
    unlink(socket_name);
    pthread_mutex_destroy(&clients_mutex);
}

void remove_client(int i) {
    for(int j = i; j < last_client; j++) {
        if(j < MAX_CLIENTS) {
            memcpy(client_names[j] , client_names[j+1], MAX_NAME_LEN);
            clients[j] = clients[j+1];
            client_states[j] = client_states[j+1];
            pings[j] = pings[j+1];
        }
    }
    clients[last_client] = 0;
    pings[last_client] = 0;
    client_states[last_client--] = 0;
}

int client_from_fd(int sockfd) {
    for(int j = 0; j < last_client; j++) {
        if(clients[j] == sockfd) {
            return j;
        }
    }
    return 0;
}

void handle_received(int sockfd, Type type, void *data) {
    pthread_mutex_lock(&clients_mutex);
    int client_id = client_from_fd(sockfd);
    switch(type) {
        case RESULT: {
            Result *result = (Result*) data;
            printf("File %s contains %d words (Counted by client: %s, number: %d)\n", result->name, result->words, client_names[client_id], result->number);
            client_states[client_id] = 0;
            Type type;
            void *data;
            receive_message(sockfd, &type, &data);
            if(type == RESULT)
                printf("%s", (char *) data); 
            client_working[client_id] = 0;
            break;
        }
        case NAME: {
            if(*((char*) data) != 0) {
                for(int i = 0; i < last_client - 1; i++) {
                    if(strncmp(client_names[i], data, MAX_NAME_LEN) == 0) {
                        send_message(sockfd, NAME_TAKEN, 0, NULL);
                        remove_client(client_id);
                        close(sockfd);
                        pthread_mutex_unlock(&clients_mutex);
                        return;
                    }
                }
                printf("Client %s registered\n", (char *) data);
                send_message(sockfd, NAME, 0, NULL);
                memcpy(client_names[client_id], data, MAX_NAME_LEN);
                pings[client_id] = 1;
            }
            break;
        }
        case DEREGISTER: {
            printf("Client %s deregistered\n", client_names[client_id]);
            close(sockfd);
            remove_client(client_id);
            break;
        }
        case PING: {
            pings[client_id] = 1;
            break;
        }
        case TEXT:
            break;
        case NAME_TAKEN:
            break;
    }
    pthread_mutex_unlock(&clients_mutex);
}


void add_client(int sockfd, int epoll_fd) {
    if((clients[last_client] = accept(sockfd, NULL, NULL))  > 0) { 
        struct epoll_event event = {
            .events = EPOLLIN,
            .data.fd = clients[last_client]
        };
        if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clients[last_client], &event))
            error_exit("Epoll clt failed");
        last_client++;
    } else {
        error_exit("Failed to add the client");
    }
}

void *server_run(void *args) {
    int epoll_fd = epoll_create1(0);
    struct epoll_event socket_event = {
        .events = EPOLLIN,
        .data.fd = sockfd
    };
    struct epoll_event socket_event2 = {
        .events = EPOLLIN,
        .data.fd = sockfd2
    };
    struct epoll_event events[MAX_CLIENTS];
    if(epoll_fd == -1) 
        error_exit("Epoll failed");
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &socket_event))
        error_exit("Epoll clt failed");
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd2, &socket_event2))
        error_exit("Epoll clt failed");
    Type type;
    while(1) {
        int event_count = epoll_wait(epoll_fd, events, MAX_CLIENTS, -1);
        for(int i = 0; i < event_count; i++) {
            if(events[i].data.fd == sockfd) {
                pthread_mutex_lock(&clients_mutex);
                add_client(sockfd, epoll_fd);
                pthread_mutex_unlock(&clients_mutex);
            } else if(events[i].data.fd == sockfd2) {
                pthread_mutex_lock(&clients_mutex);
                add_client(sockfd2, epoll_fd);
                pthread_mutex_unlock(&clients_mutex);
            } else {
                void *data;
                if(receive_message(events[i].data.fd, &type, &data) > -1) {
                    handle_received(events[i].data.fd, type, data);
                    free(data);
                } else {
                    int client_id = client_from_fd(events[i].data.fd);
                    if(client_id < last_client) 
                        printf("Client %s disconnected\n", client_names[client_id]);
                    close(events[i].data.fd);
                    remove_client(client_from_fd(events[i].data.fd));
                }
            }
        }
    }
}


void *handle_input(void *args) {
    char input[50];
    FILE *fd;
    int counter = 0;
    while(1) {
        scanf("%s", input);
        if(strcmp(input, "list") == 0) {
            if(last_client == 0) printf("No active clients\n");
            for(int i = 0; i < last_client; i++) {
                printf("Client %d, name: %s\n", i, client_names[i]);
            }
        } else {
            if((fd = fopen(input, "r")) == NULL) {
                printf("file %s doesn't exist\n", input);
            } else {
                pthread_mutex_lock(&clients_mutex);
                fseek(fd, 0, SEEK_END);
                ssize_t size = ftell(fd) + MAX_NAME_LEN;
                char *data = malloc(size);
                sprintf(data, "%s", input);
                fseek(fd, 0, SEEK_SET);
                memcpy(data + MAX_NAME_LEN, &counter, sizeof(int));
                for(int i = 0; i < last_client; i++) {
                    if(client_states[i] == 0 || i == last_client -1) {
                        client_states[i] = 1;
                        fread(data + sizeof(int) + MAX_NAME_LEN, 1, size, fd);
                        client_working[i] = 1;
                        send_message(clients[i], TEXT, size + 3, data); 
                        break;
                    }   
                }
                counter++;
                pthread_mutex_unlock(&clients_mutex);
                free(data);
            }
        }
    }
}

void *ping(void *args) {
    while(1) {
        pthread_mutex_lock(&clients_mutex);
        for(int i = 0; i < last_client; i++) {
            send_message(clients[i], PING, 0, NULL);
        }
        pthread_mutex_unlock(&clients_mutex);
        sleep(1);
        pthread_mutex_lock(&clients_mutex);
        for(int i = 0; i < last_client; i++) {
            if(pings[i] == 0 && client_working[i] == 0) {
                printf("Client %s did not respond, disconnecting\n", client_names[i]);
                remove_client(i);
                close(clients[i]);
                i--;
            } else {
                pings[i] = 0;
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }
}

int main(int argc, char **argv) {
    if(argc < 3) {
        fprintf(stderr, "Usage: ./server [TCP PORT ADDRESS] [SOCKET NAME]");
        exit(-1);
    }
    signal(SIGINT, handle_int);
    signal(SIGPIPE, SIG_IGN);
    atexit(handle_exit);
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    memcpy(addr.sun_path, argv[2], strlen(argv[2]));
    memcpy(socket_name, argv[2], strlen(argv[2]));
    const struct sockaddr_in addr2 = {
        .sin_family = AF_INET,
        .sin_port = htons(atoi(argv[1])),
        .sin_addr = {
            .s_addr = htonl(INADDR_ANY)
        }
    };
    sockfd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    sockfd2 = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if(sockfd == -1 || sockfd2 == -1)
        error_exit("Failed to create the socket");
    if(bind(sockfd, (const struct sockaddr*) &addr, sizeof(addr)) != 0)
        error_exit("Bind failure");
    if(bind(sockfd2, (const struct sockaddr*) &addr2, sizeof(addr2)) != 0)
        error_exit("Bind failure");
    if(listen(sockfd, 20) != 0 || listen(sockfd2, 20) != 0)
        error_exit("Listen failure");
    if(pthread_mutex_init(&clients_mutex, NULL) != 0)
        error_exit("Mutex failure");
    pthread_t server_thread, input_thread, ping_thread;
    pthread_create(&server_thread, NULL, server_run, NULL);
    pthread_create(&input_thread, NULL, handle_input, NULL);
    pthread_create(&ping_thread, NULL, ping, NULL);
    pthread_join(server_thread, NULL);
    pthread_join(input_thread, NULL);
    pthread_join(ping_thread, NULL);
}