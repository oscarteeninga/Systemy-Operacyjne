#include "config.h"

int last_client_id = 0;
mqd_t mqd;
mqd_t clients[100];
int friends_num = 0;
int friends[100];

void handle_init(int);
void handle_echo(int, int);
void handle_to_one(int, int);
void handle_list();
void handle_to_all();
void handle_friends(int);
void handle_add_friends(int);
void handle_del_friends(int);
void handle_to_friends(int, int);
void handle_stop(int);
void handle_received(int, int, int);
void int_handler(int);
void on_quit();

int main() {
    text = calloc(sizeof(char), MAX_MESSAGE_LEN);
    signal(SIGINT, int_handler);
    struct mq_attr attr;
    attr.mq_maxmsg = MAX_MSG_NUM;
    attr.mq_msgsize = MSG_SIZE;
    if((mqd = mq_open(SERVER, O_RDONLY | O_CREAT | O_EXCL, 0666, &attr)) == -1) die("msgget");
    atexit(on_quit);
    int type, int_val, text_len;
    while(1) {
        receive_data(mqd, &type, &int_val, &text_len);
        handle_received(type, int_val, text_len);
    }
}

void die(char *msg) {
    perror(msg);
    exit(errno);
}

void handle_init(int key) {
    char path[15];
    sprintf(path, "/%d", key);
    if((clients[last_client_id] = mq_open(path, O_WRONLY)) == -1) die("mq_open");
    send_data(clients[last_client_id], INIT, last_client_id, 0);
    last_client_id++;
}

void handle_echo(int client_id, int text_len) {
    time_t now;
    time(&now);
    size_t len = sprintf(text, "%.*s%s", text_len, text, ctime(&now));
    send_data(clients[client_id], ECHO, 0, len);
}

void handle_to_one(int client_id, int text_len) {
    if(clients[client_id] != 0) {
        time_t now;
        time(&now);
        size_t len = sprintf(text, "%.*s%sFrom ID: %d\n", text_len, text, ctime(&now), client_id);
        send_data(clients[client_id], ECHO, 0, len);
    }   
}

void handle_list() {
    printf("Client list:\n");
    for(int i = 0; i < last_client_id; i++) {
        if(clients[i] != 0) printf("ID: %d, QUEUE: %d\n", i, clients[i]);
    }
}

void handle_to_all(int client_id, int text_len) {
    time_t now;
    time(&now);
    size_t len = sprintf(text, "%.*s%sFrom ID: %d\n",text_len, text, ctime(&now), client_id);
    for(int i = 0; i < last_client_id; i++) {
        if(clients[i] != 0) send_data(clients[i], ECHO, 0, len);
    }
}

void handle_friends(int text_len) {
    char *tmp = malloc(text_len);
    memcpy(tmp, text, text_len);
    char *str = strtok(tmp, " ");
    friends_num = 0;
    while(str != NULL) {
        int exists = 0;
        int num = atoi(str);
        for(int i = 0; i < friends_num; i++) {
            if(friends[i] == num) exists = 1;
        }
        if(!exists) friends[friends_num++] = num;
        str = strtok(NULL, " ");
    }
}

void handle_add_friends(int text_len) {
    char *tmp = malloc(text_len);
    memcpy(tmp, text, text_len);
    char *str = strtok(tmp, " ");
    while(str != NULL) {
        int exists = 0;
        int num = atoi(str);
        for(int i = 0; i < friends_num; i++) {
            if(friends[i] == num) exists = 1;
        }
        if(!exists) friends[friends_num++] = num;
        str = strtok(NULL, " ");
    }
}

void handle_del_friends(int text_len) {
    char *tmp = malloc(text_len);
    memcpy(tmp, text, text_len);
    char *str = strtok(tmp, " ");
    while(str != NULL) {
        int num = atoi(str);
        for(int i = 0; i < friends_num; i++) {
            if(friends[i] == num) {
                for(int j = i; j < friends_num - 1; j++) friends[j] = friends[j+1];
                friends_num--;
            }
        }
        str = strtok(NULL, " ");
    }
}

void handle_to_friends(int client_id, int text_len) {
    time_t now;
    time(&now);
    size_t len = sprintf(text, "%.*s%sFrom ID: %d\n",text_len, text, ctime(&now), client_id);
    for(int i = 0; i < friends_num; i++) {
        if(clients[friends[i]] != 0) send_data(clients[friends[i]], ECHO, 0, len);
    }
}

void handle_stop(int client_id) {
    clients[client_id] = 0; 
    int active_clients = 0;
    for(int i = 0; i < last_client_id; i++) {
        if(clients[i] != 0) active_clients++;
    }
    if(active_clients == 0) {
        printf("No active clients\n");
        exit(0);
    }
}

void handle_received(int type, int int_val, int text_len) {
    switch(type) {
        case INIT:
            handle_init(int_val);
            break;
        case ECHO:
            handle_echo(int_val, text_len);
            break;
        case TO_ONE:
            handle_to_one(int_val, text_len);
            break;
        case LIST:
            handle_list();
            break;
        case TO_ALL:
            handle_to_all(int_val, text_len);
            break;
        case FRIENDS:
            handle_friends(text_len);
            break;
        case ADD_FRIENDS:
            handle_add_friends(text_len);
            break;
        case DEL_FRIENDS:
            handle_del_friends(text_len);
            break;
        case TO_FRIENDS:
            handle_to_friends(int_val, text_len);
            break;
        case STOP:
            handle_stop(int_val);
            break;
    }
}

void int_handler(int signum) {
    printf("\nInterrupt signal\n");
    exit(0);
}

void on_quit() {
    for(int i = 0; i < last_client_id; i++) {
        if(clients[i] != 0) {
            send_data(clients[i], STOP, 0, 0);
            mq_close(clients[i]);
            receive_data(mqd, NULL, NULL, NULL);
        }
    }
    mq_close(mqd);
    mq_unlink(SERVER);
}

