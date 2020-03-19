#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#define KEY  "./queuekey"

struct message {
    long mtype;
    long value;
};

int main() {
    sleep(1);
    int val = 0;

	/**********************************
	Otworz kolejke systemu V "reprezentowana" przez KEY
	**********************************/
    int q = msgget(ftok(KEY, 0), IPC_CREAT | 0666);
	    /**********************************
	    Odczytaj zapisane w kolejce wartosci i przypisz je do zmiennej val
	    obowiazuja funkcje systemu V
	    ************************************/
       struct message msg;
       msgrcv(q, &msg, sizeof(struct message), 0, 0);
       val = msg.value;
       printf("%d square is: %d\n", val, val*val);

	/*******************************
	posprzataj
	********************************/
    msgctl(q, IPC_RMID, 0);
    return 0;
}
