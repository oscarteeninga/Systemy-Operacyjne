#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <semaphore.h>
#include <sys/mman.h>

#include "tasma.h"
#include "config.h"

sem_t *semaphores[3];

char *home() {
    char *home = getenv("HOME");
    if (!home) die("$HOME is not set");
    return home;
}

key_t get_key() {
    key_t key = ftok(home(), PROJECT_ID);
    if (key == -1) die("key");
    return key;
}

belt *get_belt() {
    int shm_id = shm_open("/belt", O_RDWR, 0);
    if (shm_id == -1) die("shm_open");

    int size;
    if (read(shm_id, &size, sizeof(int)) == -1) die("read");

    belt *b = mmap(NULL, sizeof(belt) + size*sizeof(package),
                    PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
    if (b == (void*)-1) die("mmap");

    close(shm_id);
    
    if ((semaphores[BUFF] = sem_open("/buff", O_RDWR)) == SEM_FAILED) die("sem_open");
    if ((semaphores[EMPTY] = sem_open("/empty", O_RDWR)) == SEM_FAILED) die("sem_open");
    if ((semaphores[FILL] = sem_open("/fill", O_RDWR)) == SEM_FAILED) die("sem_open");

    return b;
}

belt *create_belt(int size, int max_weight) {
    int shm_id = shm_open("/belt", O_RDWR | O_CREAT | O_EXCL, 0666);
    if (shm_id == -1) die("shm_open");

    if (ftruncate(shm_id, sizeof(belt) + size * sizeof(package)) == -1) die("fturncate");

    belt *b = mmap(NULL, sizeof(belt) + size * sizeof(package),
                    PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
    
    if (b == (void*)-1) die("mmap");

    close(shm_id);

    b->size = size;
    b->first = 0;
    b->last = 0;
    b->weight = 0;
    b->max_weight = max_weight;
    b->trucker_done = 0;
    b->emptied = 0;
    
    if ((semaphores[BUFF] = sem_open("/buff", O_RDWR | O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) die("sem_open");
    if ((semaphores[EMPTY] = sem_open("/empty", O_RDWR | O_CREAT | O_EXCL, 0666, b->size)) == SEM_FAILED) die("sem_open");
    if ((semaphores[FILL] = sem_open("/fill", O_RDWR | O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) die("sem_open");

    return b;
}

void delete_belt(belt *b) {
    sem_unlink("/buff");
    sem_unlink("/empty");
    sem_unlink("/fill");
    shm_unlink("/belt");
}

void timestamp_package(package *pkg) {
    gettimeofday(&pkg->loaded_at, NULL);
}

package create_package(int weight) {
    package pkg;
    pkg.weight = weight;
    pkg.loader = getpid();
    return pkg;
}

void put_package(belt *b, package pkg) {
    int size, taken, weight, max_weight;
    struct timeval start_time = curr_time();
    sleep(LOADER_INTERVAL);
    int done = 0, first_try = 1, first_try_failed = 0;
    while (!done) {
        if (b->trucker_done) {
            printf("Trucker został zamknięty\n");
            exit(0);
        }
        if (first_try_failed == 2) {
            print_current_time();
            printf(", PID:%d, oczekiwanie na taśmę w celu załadowania %dkg paczki..\n", getpid(), pkg.weight);
        } else if (first_try_failed == 1) first_try_failed = 0;

        if (first_try) {
            first_try = 0;
            if (dec_sem_nonblock(b, EMPTY) == -1) {
                first_try_failed = 1;
                dec_sem(b, EMPTY);
            }
        } else {
            dec_sem(b, EMPTY);
        }
        dec_sem(b, BUFF);

        if (b->max_weight >= b->weight + pkg.weight && !b->trucker_done) {
            timestamp_package(&pkg);
            b->buffer[b->last] = pkg;
            b->last = (b->last + 1) % b->size;
            b->weight += pkg.weight;

            size = b->size;
            taken = (b->last + size - 1 - b->first) % size + 1;

            weight = b->weight;
            max_weight = b->max_weight;

            done = 1;
        }

        inc_sem(b, BUFF);
        if (done) {
            inc_sem(b, FILL);
        } else {
            inc_sem(b, EMPTY);
            if (first_try_failed == 1) first_try_failed = 2;
        }
    }

    print_time(pkg.loaded_at);
    printf(", PID: %d, %dkg paczka połozona po %ldus, taśma: %d/%d %dkg/%dkg\n",
        pkg.loader,
        pkg.weight,
        time_diff(start_time, curr_time()),
        taken,
        size,
        weight,
        max_weight);
}

void get_package(belt *b, truck *t) {
    int size, taken, weight, max_weight;
    sleep(TRUCKER_INTERVAL);
    package pkg;
    int done = 0, first_try = 1;
    while(!done) {
        if (first_try) {
            first_try = 0;
            if (dec_sem_nonblock(b, FILL) == -1) {
                if (b->trucker_done) exit(0);
                printf(", oczekiwanie na paczkę...\n");
                dec_sem(b, FILL);
            }
        } else {
            dec_sem(b, FILL);
        }
        dec_sem(b, BUFF);

        pkg = b->buffer[b->first];
        if (pkg.weight <= t->capacity) {
            b->first = (b->first + 1) % b->size;
            b->weight -= pkg.weight;
            t->capacity -= pkg.weight;

            size = b->size;
            taken = (b->last + size - b->first) % size;
            
            weight = b->weight;
            max_weight = b->max_weight;

            done = 1;
        }

        inc_sem(b, BUFF);
        if (done) {
            inc_sem(b, EMPTY);
        } else {
            inc_sem(b, FILL);
        }

        if(!done) {
            print_current_time();
            printf(", ciezarowka pełna!\n");
            print_current_time();
            printf(", najdechała nowa cięarowka...\n");
            t->capacity = t->max_capacity;
        }
    }

    print_current_time();
    printf(", PID %d, %dkg paczka odebrana i załadowana po %ldus, %dkg wolne, taśma: %d/%d %dkg/%dkg\n",
        pkg.loader,
        pkg.weight,
        time_diff(pkg.loaded_at, curr_time()),
        t->capacity,
        taken,
        size,
        weight,
        max_weight);

    if (b->trucker_done && size == 0) exit(0);
}

void inc_sem(belt *b, int sem_num) {
    while (sem_post(semaphores[sem_num]) == -1) {
        if (errno != EINTR || b->emptied) exit(0);
    }
}

void dec_sem(belt *b, int sem_num) {
    while (sem_wait(semaphores[sem_num]) == -1) {
        if (errno != EINTR || b->emptied) exit(0);
    }
}

int dec_sem_nonblock(belt *b, int sem_num) {
    if (sem_trywait(semaphores[sem_num]) == -1) {
        if (errno == EAGAIN || (errno == EINTR && !b->emptied)) return -1;
        else exit(0);
    } 
    return 0;
}