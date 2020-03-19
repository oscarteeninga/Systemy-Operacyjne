#include "main.h"

int main(int argc, char *argv[]) {
    if (argc < 6) die("Not enought arguments!");

    threads_num = atoi(argv[1]);
    char *mode = argv[2];
    char *input = argv[3];
    char *filter_file = argv[4];
    char *output = argv[5];


    image = read_image(input);
    fil = read_filter(filter_file);
    newbie = create_empty_image(image->width, image->height);
    filter_parallel(mode);
    save_image(newbie, output);
    return 0;
}

void die(char *msg) {
    perror(msg);
    exit(-1);
}

Image *read_image(char *name) {
    FILE *fd = fopen(name, "r");
    if (!fd) die("fopen");
    char *line = calloc(20, sizeof(char));
    size_t size = 2;
    if (getline(&line, &size, fd) == -1) die("getline");
    if (strncmp(line, "P2", 2) != 0) die("strncmp");
    size = 20;
    if (getline(&line, &size, fd) == -1) die("getline");
    Image *image = calloc(1, sizeof(Image));
    if (sscanf(line, "%d %d\n", &(image->width), &(image->height)) == 0) die("sscanf");
    if (getline(&line, &size, fd) == -1) die("getline");
    int gray;
    sscanf(line, "%d\n", &gray);
    if (gray > 255) die("argument");
    image->data = calloc(image->height, sizeof(char*));
    char *value = calloc(3, sizeof(char));
    for (int i = 0; i < image->height; i++) {
        image->data[i] = calloc(image->width, sizeof(char));
        for (int j = 0; j < image->width; j++) {
            if (fread(value, sizeof(char), 1, fd) == 0) die("fread");
            if (value[0] == '\n') {
                if (fread(value, sizeof(char), 1, fd) == 0) die("fread");
            }
            if (fread(value + 1, sizeof(char), 2, fd) == 0) die("fread");
            image->data[i][j] = atoi(value);
            if (fread(value, sizeof(char), 1, fd) == 0) die("fread");
        }
    }
    fclose(fd);
    return image;
}

Image *create_empty_image(int width, int height){
    Image *image = calloc(1, sizeof(image));
    image->data = calloc(height, sizeof(char*));
    image->width = width;
    image->height = height;
    for (int i = 0; i < image->height; i++) image->data[i] = calloc(width, sizeof(char));
    return image;
}

void save_image(Image *image, char *name) {
    FILE *fd = fopen(name, "w+");
    if (!fd) die("fopen");
    char *line = calloc(20, sizeof(char));
    if (fwrite("P2\n", 1, 3, fd) == 0) die("fwrite");
    int length = sprintf(line, "%d %d\n", image->width, image->height);
    if (length == 0) die("sprintf");
    if (fwrite(line, 1, length, fd) == 0) die("fwrite");
    if (fwrite("255\n", 1, 4, fd) == 0) die("fwrite");
    int counter = 0;
    // i - height, j - width
    for (int i = 0; i < image->height; i++) {
        for (int j = 0; j < image->width; j++) {
            counter++;
            if (counter < 17) fprintf(fd, "%3u ", image->data[i][j]);
            else {
                counter = 0;
                fprintf(fd, "%3u \n", image->data[i][j]);
            }
        }
    }
    fclose(fd);
}

void filter(Image *new, Image *image, Filter *filter, int x, int y) {
    double sum = 0;
    for (int i = 0; i < filter->size; i++) {
        for (int j = 0; j < filter->size; j++) {
            sum += image->data[(int) (fmax(1, fmin(image->height - 1, x - ceil(filter->size/2) + i + 1)))]
                              [(int) (fmax(1, fmin(image->width - 1, y - ceil(filter->size/2) + i + 1)))] * filter->data[i][j];
        }
    }
    new->data[x][y] = (unsigned char) round(sum);
}

Filter *read_filter(char *name) {
    FILE *fd = fopen(name, "r");
    if (!fd) die("fopen");
    char *line = calloc(20, sizeof(char));
    size_t size = 20;
    if (getline(&line, &size, fd) == -1) die("getline");
    Filter *filter = calloc(1, sizeof(Filter));
    filter->size = atoi(line);
    filter->data = calloc(filter->size, sizeof(float*));
    char *value = calloc(10, sizeof(char));

    for (int i = 0; i < filter->size; i++) {
        filter->data[i] = calloc(filter->size, sizeof(float));
        size_t size = 10;
        for (int j = 0; j < filter->size; j++) {
            getdelim(&value, &size, ' ', fd);
            filter->data[i][j] = strtof(value, NULL);
        }
    }
    fclose(fd);
    return filter;
}

void *filter_block(void *ptr) {
    int k = *((int*) ptr);
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for (int i = k*ceil(range); i < (k+1)*ceil(range); i++) {
        for (int j = 0; j < image->height; j++) {
            filter(newbie, image, fil, j, i);
        }
    }
    gettimeofday(&end, NULL);
    int *elapsed = calloc(1, sizeof(int));
    *elapsed = ((end.tv_sec - start.tv_sec) * 1000000) + (end.tv_usec - start.tv_usec);
    pthread_exit((void*) elapsed);
}

void *filter_intervaled(void *ptr) {
    int k = *((int*) ptr);
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for (int i = k; i < image->width; i+= threads_num) {
        for (int j = 0; j < image->height; j++) {
            filter(newbie, image, fil, j, i);
        }
    }
    gettimeofday(&end, NULL);
    int *elapsed = calloc(1, sizeof(int));
    *elapsed = ((end.tv_sec - start.tv_sec) * 1000000) + (end.tv_usec - start.tv_usec);
    pthread_exit((void*) elapsed);
}

void filter_parallel(char *mode) {
    range = image->width/threads_num;
    threads = calloc(threads_num, sizeof(pthread_t));
    struct timeval start, end;
    gettimeofday(&start, NULL);
    int times[threads_num];
    for (int i = 0; i < threads_num; i++) {
        int *a = calloc(1, sizeof(int));
        memcpy(a, &i, sizeof(int));
        if (strcmp(mode, "block") == 0) {
            pthread_create(&threads[i], NULL, filter_block, a);
        } else if (strcmp(mode, "intervaled") == 0) {
            pthread_create(&(threads[i]), NULL, filter_intervaled, a);
        } else die("mode");
    }

    for (int i = 0; i < threads_num; i++) {
        int *retval;
        if (pthread_join(threads[i], (void**) &retval) != 0) die("pthread_join");
        times[i] = *retval;
        free(retval);
    }

    gettimeofday(&end, NULL);
    for (int i = 0; i < threads_num; i++) printf("Thread %d, TID: %d, time: %d us\n", i, (int) threads[i], times[i]);
    int elapsed = ((end.tv_sec - start.tv_sec) * 1000000) + (end.tv_usec - start.tv_usec);
    printf("PID: %d, Total time: %d\n", getpid(), elapsed);
}
