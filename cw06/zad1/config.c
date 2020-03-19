#include "config.h"

void send_data(int id, int type, int int_val, int text_len) {
    Message message;
    message.mtype = type;
    message.int_val = int_val;
    message.text_size = text_len;
    if(text_len > MAX_MESSAGE_LEN) {
        fprintf(stderr, "Message too long\n");
        exit(-1);
    }
    if(text_len > 0) {
        memcpy(message.mtext, text, text_len);
    }   
    if(msgsnd(id, &message, MSG_SIZE, 0) == -1) die("msgsnd");
}

void receive_data(int id, int *type, int *int_val, int *text_len) {
    Message message;
    if(msgrcv(id, &message, MSG_SIZE, 0, 0) == -1) die("msgrcv");
    if(type != NULL) *type = message.mtype;
    if(int_val != NULL) *int_val = message.int_val;
    if(text_len != NULL) { 
        *text_len = message.text_size;
        if((*text_len) > 0) {
            text = memcpy(text, message.mtext, (*text_len));
        }   
    }
}