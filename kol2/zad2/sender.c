#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>

#define SHM_NAME "/kol_shm"
#define MAX_SIZE 1024

int main(int argc, char **argv) {
    /***************************************
    Utworz/otworz posixowy obszar pamieci wspolnej "reprezentowany" przez SHM_NAME
    zapisz tam wartosc przekazana jako parametr wywolania programu
    posprzataj
    *****************************************/
    int shm_id = shm_open(SHM_NAME, O_CREAT | O_RDWR | 0666);
    ftruncate(shm_id, MAX_SIZE);
    int *tmp = mmap(NULL, MAX_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
    *tmp = atoi(argv[1]);
    munmap(&shm_id, MAX_SIZE);
    return 0;
}
