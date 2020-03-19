#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/types.h>
#include <pthread.h>

void *hello(void *arg) {
    while (1) {
        printf("Hello world from thread number %d\n", *(int *) arg);
        /****************************************************
            przerwij dzialanie watku jesli bylo takie zadanie
        *****************************************************/
        pthread_exit(arg);
        printf("Hello again world from thread number %d\n", *(int *) arg);
        sleep(2);
    }
    return NULL;
}


int main(int argc, char **args) {
    if (argc != 3) {
        printf("Not a suitable number of program parameters\n");
        return (1);
    }
    int n = atoi(args[1]);
    /**************************************************
        Utworz n watkow realizujacych funkcje hello
    **************************************************/
    pthread_t *hello_threads[n];

    for (int i = 0; i < n; i++) {
        int *retval = calloc(1, sizeof(int));
        memcpy(retval, &i, sizeof(int));
        pthread_create(&hello_threads[i], NULL, hello, retval);
    }

    int i = 0;
    while (i++ < atoi(args[2])) {
        printf("Hello from main %d\n", i);
        sleep(0);
    }

    int hello_returns[n];

    for (int i = 0; i < n; i++) {
        void *retval;
        pthread_join(hello_threads[i], &retval);
        hello_returns[i] = *(int*)retval;
    }

    for(int i = 0; i < n; i++) {
        printf("Thread: %d, retval: %d\n", (int) hello_threads[i], (int) hello_returns[i]);
    }
    /*******************************************
        "Skasuj" wszystke uruchomione watki i poczekaj na ich zakonczenie
    *******************************************/
    printf("DONE");
    return 0;
}

