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
    int val = 0;
    /*******************************************
    Utworz/otworz posixowy obszar pamieci wspolnej "reprezentowany" przez SHM_NAME
    odczytaj zapisana tam wartosc i przypisz ja do zmiennej val
    posprzataj
    *********************************************/
    int shm_id = shm_open(SHM_NAME, O_CREAT | O_RDWR | 0666);
    int *tmp = mmap(NULL, MAX_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
    val = *tmp;
    printf("%d square is: %d \n", val, val * val);
    munmap(&shm_id, MAX_SIZE);
    shm_unlink(SHM_NAME);
    return 0;
}
