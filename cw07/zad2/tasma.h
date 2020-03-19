#include <sys/types.h>
#include <sys/time.h>
#include <semaphore.h>

typedef struct package {
    int weight;
    pid_t loader;
    struct timeval loaded_at;
} package;

typedef struct belt {
    int size;
    int first; 
    int last;
    int max_weight;
    int weight;
    int trucker_done;
    int emptied;
    package buffer[];
} belt;

typedef struct truck {
    int capacity;
    int max_capacity;
} truck;

enum sem_opt { BUFF = 0, EMPTY = 1, FILL = 2 };

belt *get_belt();
belt *create_belt(int, int);
void delete_belt(belt*);

package create_package(int);
void timestamp_package(package*);

void put_package(belt*, package);
void get_package(belt*, truck*);

void inc_sem(belt*, int);
void dec_sem(belt*, int);
int dec_sem_nonblock(belt*, int);
