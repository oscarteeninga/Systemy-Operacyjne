#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

typedef struct Passenger {
    int id;
    int cart;
} Passenger;

typedef struct Cart {
    int id;
    int size;
    int raids;
    Passenger *passengers;
} Cart;

int carts_count;
int cart_size;

pthread_cond_t *carts_cond;
pthread_mutex_t *carts_mutex;

pthread_mutex_t station_mutex;
pthread_mutex_t passenger_mutex;
pthread_mutex_t empty_cart_mutex;
pthread_mutex_t full_cart_mutex;

pthread_cond_t empty_condition;
pthread_cond_t full_condition;

Cart *carts;
int station_cart_id;

struct timeval get_curr_time();
void *run_passenger(void*);
void *run_cart(void*);
void die(char*);

int main(int argc, char *argv[]) {
    if (argc < 5) die("Not enought arguments");

    int passengers_count = atoi(argv[1]);
    carts_count = atoi(argv[2]);
    cart_size = atoi(argv[3]);
    int raid_count = atoi(argv[4]);

    if (passengers_count <= 0 || carts_count <= 0 || cart_size <= 0 || raid_count <= 0) die("Args need to be positive");

    station_cart_id = 0;
    pthread_t pass_thr[passengers_count];
    pthread_t cart_thr[carts_count];
    Passenger passengers[passengers_count];
    carts = malloc(sizeof(Cart) * carts_count + sizeof(int) * passengers_count);
    carts_mutex = malloc(sizeof(pthread_mutex_t) * carts_count);
    carts_cond = malloc(sizeof(pthread_cond_t) * carts_count);

    pthread_mutex_init(&station_mutex, NULL);
    pthread_mutex_init(&empty_cart_mutex, NULL);
    pthread_mutex_init(&passenger_mutex, NULL);
    pthread_mutex_init(&full_cart_mutex, NULL);
    pthread_cond_init(&empty_condition, NULL);
    pthread_cond_init(&full_condition, NULL);

    for (int i = 0; i < passengers_count; i++) {
        passengers[i].id = i;
        passengers[i].cart = -1;
    }

    for (int i = 0; i < carts_count; i++) {
        carts[i].id = i;
        carts[i].size = 0;
        carts[i].raids = raid_count;
        carts[i].passengers = malloc(sizeof(Passenger) * cart_size);
        pthread_mutex_init(&carts_mutex[i], NULL);
        pthread_cond_init(&carts_cond[i], NULL);
    }

    for (int i = 0; i < carts_count; i++) {
        pthread_create(&cart_thr[i], NULL, run_cart, &carts[i]);
    }

    for (int i = 0; i < passengers_count; i++) {
        pthread_create(&pass_thr[i], NULL, run_passenger, &passengers[i]);
    }

    for (int i = 0; i < carts_count; i++) {
        pthread_join(cart_thr[i], NULL);
    }

    for (int i = 0; i < carts_count; i++) {
        pthread_mutex_destroy(&carts_mutex[i]);
    }

    for(int i = 0; i < carts_count; i++){
        free(carts[i].passengers);
    }

    free(carts);
    free(carts_mutex);
    free(carts_cond);
}

void die(char *msg) {
    perror(msg);
}

struct timeval get_curr_time() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time;
}

void *run_passenger(void *data) {
       Passenger *passenger = (Passenger *) data;
    while (1) {
        pthread_mutex_lock(&passenger_mutex);

        passenger->cart = station_cart_id;
        carts[station_cart_id].passengers[carts[station_cart_id].size] = *passenger;
        carts[station_cart_id].size = carts[station_cart_id].size + 1;
        struct timeval curr = get_curr_time();
        printf("Passenger %d entered the cart: %d, people inside: %d, time: %ld.%06ld \n", 
            passenger->id, station_cart_id, carts[station_cart_id].size, curr.tv_sec, curr.tv_usec);

        if(carts[station_cart_id].size == cart_size){
            srand(time(NULL));
            curr = get_curr_time();
            printf("Passenger %d pressed start in cart: %d, people inside: %d, time: %ld.%06ld\n", 
                carts[station_cart_id].passengers[rand()%carts->size].id, station_cart_id, carts[station_cart_id].size, curr.tv_sec, curr.tv_usec);
            pthread_cond_signal(&full_condition);
            pthread_mutex_unlock(&full_cart_mutex);
        } else {
            pthread_mutex_unlock(&passenger_mutex);
        }

        pthread_mutex_lock(&carts_mutex[passenger->cart]);

        curr = get_curr_time();
        carts[station_cart_id].size--;
        printf("Passenger %d left cart: %d, people in cart: %d, time: %ld.%06ld \n", 
            passenger->id, station_cart_id, carts[station_cart_id].size,curr.tv_sec, curr.tv_usec);

        if(carts[station_cart_id].size == 0){
            pthread_cond_signal(&empty_condition);
            pthread_mutex_unlock(&empty_cart_mutex);
        }

        pthread_mutex_unlock(&carts_mutex[passenger->cart]);
        passenger->cart = -1;
    }
}

void *run_cart(void *data) {
    Cart *cart = (Cart *) data;
    if (cart->id == 0) {
        pthread_mutex_lock(&passenger_mutex);
    }

    for (int i = 0; i < cart->raids; i++) {
        pthread_mutex_lock(&station_mutex);

        if (cart->id != station_cart_id) {
            pthread_cond_wait(&carts_cond[cart->id], &station_mutex);
        }

        struct timeval curr = get_curr_time();
        printf("Cart %d arrived, raid: %d, time: %ld.%06ld\n", cart->id, i+1, curr.tv_sec, curr.tv_usec);

        if (i != 0) {
            pthread_mutex_unlock(&carts_mutex[cart->id]);
            pthread_cond_wait(&empty_condition, &empty_cart_mutex);
        }

        pthread_mutex_lock(&carts_mutex[cart->id]);
        pthread_mutex_unlock(&passenger_mutex);
        pthread_cond_wait(&full_condition, &full_cart_mutex);

        curr = get_curr_time();
        printf("Cart %d is full, raid: %d, time: %ld.%06ld\n", cart->id, i+1, curr.tv_sec, curr.tv_usec);

        station_cart_id = (station_cart_id + 1) % carts_count;

        pthread_cond_signal(&carts_cond[station_cart_id]);
        pthread_mutex_unlock(&station_mutex);
    }

    pthread_mutex_lock(&station_mutex);

    if (cart->id != station_cart_id) {
        pthread_cond_wait(&carts_cond[cart->id], &station_mutex);
    }

    struct timeval curr = get_curr_time();
    printf("Cart %d arrived, time: %ld.%06ld\n", cart->id, curr.tv_sec, curr.tv_usec);

    station_cart_id = cart->id;

    pthread_mutex_unlock(&carts_mutex[cart->id]);
    pthread_cond_wait(&empty_condition, &empty_cart_mutex);

    station_cart_id = (station_cart_id + 1) % carts_count;

    curr = get_curr_time();
    printf("Cart %d finished, time: %ld.%06ld\n", cart->id, curr.tv_sec, curr.tv_usec);

    pthread_cond_signal(&carts_cond[station_cart_id]);
    pthread_mutex_unlock(&station_mutex);

    pthread_exit(NULL);
}