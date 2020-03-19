#include "shared.h"

int sock;
char* name;

void init_client(char*, char*, char*);

sock_msg_t get_msg(void);
void del_msg(sock_msg_t);

void send_msg(sock_msg_t);
void send_empty(sock_msg_type_t);
void send_done(int, char*);

void int_handler(int);
void cleanup(void);

int main(int argc, char *argv[]) {
    if (argc != 4) error_exit("Pass: client name, variant (WEB/UNIX), address");

    init_client(argv[1], argv[2], argv[3]);

    while (1) {
        sock_msg_t msg = get_msg();

        switch (msg.type) {
        case OK: {
            break;
        }
        case PING: {
            send_empty(PONG);
            break;
        }
        case NAME_TAKEN: error_exit("Name is already taken");
        case FULL: error_exit("Server is full");
        case WORK: {
            puts("Doing work...");
            char *buffer = malloc(100 + 2 * msg.size);
            if (buffer == NULL) error_exit_errno();
            sprintf(buffer, "echo '%s' | awk '{for(x=1;$x;++x)print $x}' | sort | uniq -c", (char*) msg.content);
            FILE *result = popen(buffer, "r");
            if (result == 0) { free(buffer); break; }
            int n = fread(buffer, 1, 99 + 2 * msg.size, result);
            buffer[n] = '\0';
            puts("Work done...");
            send_done(msg.id, buffer);
            free(buffer);
            break;
        }
        default: break;
        }

        del_msg(msg);
    }
}

void init_client(char* n, char* variant, char* address)
{
    // register atexit
    if (atexit(cleanup) == -1) error_exit_errno();

    // register int handler
    if (signal(SIGINT, int_handler) == SIG_ERR)
        show_errno();

    // set name
    name = n;

    // parse address
    if (atoi(variant) == 0) {
        uint32_t in_addr = inet_addr("127.0.0.1");
        if (in_addr == INADDR_NONE) error_exit("Invalid address");

        uint16_t port_num = (uint16_t) atoi(address);
        if (port_num < 1024)
            error_exit("Invalid port number");

        if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
            error_exit_errno();

        struct sockaddr_in web_addr;
        memset(&web_addr, 0, sizeof(struct sockaddr_in));

        web_addr.sin_family = AF_INET;
        web_addr.sin_addr.s_addr = in_addr;
        web_addr.sin_port = htons(port_num);

        if (connect(sock, (const struct sockaddr *) &web_addr, sizeof(web_addr)) == -1)
            error_exit_errno();
    } else {
        char* un_path = address;

        if (strlen(un_path) < 1 || strlen(un_path) > UNIX_PATH_MAX)
            error_exit("Invalid unix socket path");

        struct sockaddr_un un_addr;
        un_addr.sun_family = AF_UNIX;
        snprintf(un_addr.sun_path, UNIX_PATH_MAX, "%s", un_path);

        struct sockaddr_un client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        client_addr.sun_family = AF_UNIX;
        snprintf(client_addr.sun_path, UNIX_PATH_MAX, "%s", name);

        if ((sock = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
            error_exit_errno();

        if (bind(sock, (const struct sockaddr *) &client_addr, sizeof(client_addr)) == -1)
            error_exit_errno();

        if (connect(sock, (const struct sockaddr *) &un_addr, sizeof(un_addr)) == -1)
            error_exit_errno();
    } 

    send_empty(REGISTER);
}

void send_msg(sock_msg_t msg)
{
    ssize_t head_size = sizeof(msg.type) + sizeof(msg.size) + sizeof(msg.name_size) + sizeof(msg.id);
    ssize_t size = head_size + msg.size + 1 + msg.name_size + 1;
    int8_t *buff = malloc(size);
    if (buff == NULL) error_exit_errno();

    memcpy(buff, &msg.type, sizeof(msg.type));
    memcpy(buff + sizeof(msg.type), &msg.size, sizeof(msg.size));
    memcpy(buff + sizeof(msg.type) + sizeof(msg.size), &msg.name_size, sizeof(msg.name_size));
    memcpy(buff + sizeof(msg.type) + sizeof(msg.size) + sizeof(msg.name_size), &msg.id, sizeof(msg.id));

    if (msg.size > 0 && msg.content != NULL)
        memcpy(buff + head_size, msg.content, msg.size + 1);
    if (msg.name_size > 0 && msg.name != NULL)
        memcpy(buff + head_size + msg.size + 1, msg.name, msg.name_size + 1);

    if (write(sock, buff, size) != size) error_exit_errno();

    free(buff);
}

void send_empty(sock_msg_type_t type)
{
    sock_msg_t msg = { type, 0, strlen(name), 0, NULL, name };
    send_msg(msg);
};

void send_done(int id, char *content)
{
    sock_msg_t msg = { WORK_DONE, strlen(content), strlen(name), id, content, name };
    send_msg(msg);
}

sock_msg_t get_msg(void)
{
    sock_msg_t msg;
    ssize_t head_size = sizeof(msg.type) + sizeof(msg.size) + sizeof(msg.name_size) + sizeof(msg.id);
    int8_t buff[head_size];
    if (recv(sock, buff, head_size, MSG_PEEK) < head_size)
        error_exit("Uknown message from server");

    memcpy(&msg.type, buff, sizeof(msg.type));
    memcpy(&msg.size, buff + sizeof(msg.type), sizeof(msg.size));
    memcpy(&msg.name_size, buff + sizeof(msg.type) + sizeof(msg.size), sizeof(msg.name_size));
    memcpy(&msg.id, buff + sizeof(msg.type) + sizeof(msg.size) + sizeof(msg.name_size), sizeof(msg.id));

    ssize_t size = head_size + msg.size + 1 + msg.name_size + 1;
    int8_t* buffer = malloc(size);

    if (recv(sock, buffer, size, 0) < size) {
        error_exit("Uknown message from server");
    }

    if (msg.size > 0) {
        msg.content = malloc(msg.size + 1);
        if (msg.content == NULL) error_exit_errno();
        memcpy(msg.content, buffer + head_size, msg.size + 1);
    } else {
        msg.content = NULL;
    }

    if (msg.name_size > 0) {
        msg.name = malloc(msg.name_size + 1);
        if (msg.name == NULL) error_exit_errno();
        memcpy(msg.name, buffer + head_size + msg.size + 1, msg.name_size + 1);
    } else {
        msg.name = NULL;
    }

    free(buffer);

    return msg;
}

void del_msg(sock_msg_t msg)
{
    if (msg.content != NULL)
        free(msg.content);
    if (msg.name != NULL)
        free(msg.name);
}

void int_handler(int signo)
{
    exit(0);
}

void cleanup(void)
{
    send_empty(UNREGISTER);
    unlink(name);
    shutdown(sock, SHUT_RDWR);
    close(sock);
}