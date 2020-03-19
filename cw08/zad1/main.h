#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

typedef struct {
    int width;
    int height;
    unsigned char **data;
} Image;

typedef struct {
    int size;
    float **data;
} Filter;

Image *image;
Filter *fil;
int range;
int threads_num;
Image *newbie;
pthread_t *threads;

void die(char*);

Image *read_image(char*);
Image *create_empty_image(int, int);
void save_image(Image*, char*);

void filter(Image*, Image*, Filter*, int, int);
Filter *read_filter(char*);
void *filter_block(void*);
void *filter_intervaled(void*);
void filter_parallel(char*);