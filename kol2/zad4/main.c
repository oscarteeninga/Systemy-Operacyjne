#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>

#define MAX 20
#define KEY "./path"

int main() {
    int shmid, i;
    int *buf;
    //Wygeneruj klucz dla kolejki na podstawie KEYi znaku 'a'
    //Utwórz segment pamięci współdzielonej shmid o prawach dostępu 600, rozmiarze MAX
    shmid = shmget(ftok(KEY, 'a'), MAX, IPC_CREAT | 0666);
    if (shmid == -1) {
        return 1;
    }
    //jeśli segment już istnieje, zwróć błąd, jeśli utworzenie pamięci się nie powiedzie zwróć błąd i wartość 1
    buf = shmat(shmid, NULL, 0);

    if (buf == (void*)-1) {
        return 1;
    }

    for (i = 0; i < MAX; i++) {
        buf[i] = i * i;
        printf("Wartość: %d \n", buf[i]);
    }
    //Przyłącz segment pamięci współdzielonej do procesu do buf, obsłuż błędy i zwróć 1
    printf("Memory written\n");
    //odłącz i usuń segment pamięci współdzielonej
    shmdt(buf);
    shmctl(shmid, IPC_RMID, 0);
    return 0;
}
