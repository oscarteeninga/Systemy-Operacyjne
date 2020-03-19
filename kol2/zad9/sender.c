#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define KEY "./queuekey"

struct message {
    long mtype;
    long val;
};

int main(int argc, char* argv[])
{
    if(argc !=2){
        printf("Not a suitable number of program parameters\n");
        return(1);
    }
    int q = msgget(ftok(KEY, 0), O_CREAT | 0666);
    struct message msg;
    msg.mtype = 1;
    msg.val = atoi(argv[1]);
    msgsnd(q, &msg, sizeof(struct message), 1);
    /******************************************************
     Utworz kolejke komunikatow systemu V "reprezentowana" przez KEY
    Wyslij do niej wartosc przekazana jako parametr wywolania programu
    Obowiazuja funkcje systemu V
    ******************************************************/
    return 0;
}



