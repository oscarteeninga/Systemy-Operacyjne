#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <unistd.h>
#include <time.h>

char* parse(char **, int, int);
void generate(char *file_name, int count, int length);
void sort_lib(char*, int, int);
void sort_sys(char*, int, int);
void copy_lib(char*, char*, int, int);
void copy_sys(char*, char*, int, int);
void print_time(clock_t, clock_t, char*, int, int, FILE*);

int main(int argc, char *argv[]) {
    clock_t clk;
    FILE* results = fopen("wyniki.txt", "w");

    if (argc < 2) {
        printf("Not enought arguments"); 
        exit(1);
    }

    for (int curr_arg = 1; curr_arg < argc; curr_arg++) {
        if (strcmp(argv[curr_arg],"generate") == 0) {
            char *file_name = parse(argv, ++curr_arg, argc);
            int count = atoi(parse(argv, ++curr_arg, argc));
            int length = atoi(parse(argv, ++curr_arg, argc));
            generate(file_name, count, length);
        }
        else if (strcmp(argv[curr_arg],"sort") == 0) {
            char *file_name = parse(argv, ++curr_arg, argc);
            int count = atoi(parse(argv, ++curr_arg, argc));
            int length = atoi(parse(argv, ++curr_arg, argc));
            char *type = parse(argv, ++curr_arg, argc);
            if (strcmp(type,"sys") == 0) {
                clk = clock();
                sort_sys(file_name, count, length);
                print_time(clk, clock(), "sort_sys", count, length, results);
            }
            else if (strcmp(type,"lib") == 0) {
                clk = clock();
                sort_lib(file_name, count, length);
                print_time(clk, clock(), "sort_lib", count, length, results);
            }
            else {
                printf("Unknown option");
                exit(1);
            }
        }
        else if (strcmp(argv[curr_arg],"copy") == 0) {
            char *file_name_1 = parse(argv, ++curr_arg, argc);
            char *file_name_2 = parse(argv, ++curr_arg, argc);
            int count = atoi(parse(argv, ++curr_arg, argc));
            int length = atoi(parse(argv, ++curr_arg, argc));
            char *type = parse(argv, ++curr_arg, argc);
            if (strcmp(type,"sys") == 0) {
                clk = clock();
                copy_sys(file_name_1, file_name_2, count, length);
                print_time(clk, clock(), "copy_sys", count, length, results);
            }
            else if (strcmp(type,"lib") == 0) {
                clk = clock();
                copy_lib(file_name_1, file_name_2, count, length);
                print_time(clk, clock(), "copy_lib", count, length, results);
            }
            else {
                printf("Unknown option");
                exit(1);
            }
        }
        else {
            printf("Unknown command");
        }
        struct tms end;
        times(&end);
    }
    return 0;
}

void print_time(clock_t start, clock_t end, char* command, int records, int record_size, FILE *fd) {
    printf("command %s %d %d took %f seconds to excute\n", command, records, record_size, ((double) (end - start))/CLOCKS_PER_SEC);
    fprintf(fd, "command %s %d %d took %f seconds to excute\n", command, records, record_size, ((double) (end - start))/CLOCKS_PER_SEC);
}

char *parse(char *cmd[], int curr_arg, int argc) {
    if (curr_arg >= argc) {
        printf("Not enought arguments");
        exit(1);
    }
    return cmd[curr_arg];
}

void generate(char *filename, int count, int length) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("File not found");
        exit(1);
    }

    FILE *dev_null = fopen("/dev/urandom", "r");

    srand(clock());
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < length; j++){
            fputc((char)(rand() % 26 + 65), file);
        }
    }

    fclose(dev_null);
    fclose(file);
}

void sort_lib(char *file_name, int count, int length) {
    FILE *file = fopen(file_name, "r+");
    if (!file) {
        printf("File not found");
        exit(1);
    }

    unsigned char* buffer_1 = malloc(length * sizeof(char));
    unsigned char* buffer_2 = malloc(length * sizeof(char));
    if (!buffer_1 || !buffer_2) {
        printf("Could not allocate memeory for buffers");
    }

    for (int i = 0; i < count-1; i++) {
        fseek(file, i*length, SEEK_SET);
        if (fread(buffer_1, sizeof(char), length, file) != length) {
            printf("Error reading file");
            exit(1);
        }

        unsigned char minimum = buffer_1[0];
        int min_index = i;

        for (int j = i + 1; j < count; j++) {
            fseek(file, j * length, SEEK_SET);
            unsigned char tmp = (unsigned char) fgetc(file);
            if (tmp < minimum) {
                minimum = tmp;
                min_index = j;
            }
        }
        
        fseek(file, min_index * length, SEEK_SET);

        if (fread(buffer_2, sizeof(char), length, file) != length) {
            printf("Error reading file");
            exit(1);
        }

        fseek(file, min_index * length, SEEK_SET);
        
        fwrite(buffer_1, sizeof(char), length, file);

        if (ferror(file)) {
            printf("Error writing file");
            exit(1);
        }

        fseek(file, i * length, SEEK_SET);

        fwrite(buffer_2, sizeof(char), length, file);

        if (ferror(file)) {
            printf("Error writing file");
        }
    }
    free(buffer_1);
    free(buffer_2);

    if (fclose(file) != 0) {
        printf("Error closing file");
        exit(1);
    }
}

void sort_sys(char *file_name, int count, int length) {
    int file = open(file_name, O_RDWR);
    if (!file) {
        printf("File not found");
        exit(1);
    }

    unsigned char* buffer_1 = malloc(length * sizeof(char));
    unsigned char* buffer_2 = malloc(length * sizeof(char));
    if (!buffer_1 || !buffer_2) {
        printf("Could not allocate memeory for buffers");
    }

    for (int i = 0; i < count - 1; i++) {
        lseek(file, i * length, SEEK_SET);
        if (read(file, buffer_1, length * sizeof(char)) != length) {
            printf("Error reading file");
            exit(1);
        }

        unsigned char minimum = buffer_1[0];
        int min_index = i;

        for (int j = i + 1; j < count; j++) {
            lseek(file, j * length, SEEK_SET);
            unsigned char next;
            if (read(file, &next, sizeof(char)) != 1) {
                printf("Error reading file");
                exit(1);
            }
            if (next < minimum) {
                minimum = next;
                min_index = j;
            }
        }

        lseek(file, min_index * length, SEEK_SET);

        if (read(file, buffer_2, length * sizeof(char)) != length) {
            printf("Error reading file");
            exit(1);
        }

        lseek(file, min_index * length, SEEK_SET);

        if (write(file, buffer_1, length * sizeof(char)) == -1) {
            printf("Error writing file");
            exit(1);
        }

        lseek(file, i * length, SEEK_SET);

        if (write(file, buffer_2, length * sizeof(char)) == -1) {
            printf("Error writing file");
            exit(1);
        }
    }
    free(buffer_1);
    free(buffer_2);

    if (close(file) != 0) {
        printf("Error closing file");
        exit(1);
    }
}

void copy_lib(char *file_name_1, char *file_name_2, int count, int length) {
    FILE *source = fopen(file_name_1, "r");
    if (!source) {
        printf("Error opening file");
        exit(1);
    }
    FILE *target = fopen(file_name_2, "w");
    if (!target) {
        printf("Error opening file");
        exit(1);
    }

    unsigned char * buffer = malloc(length);
    if (!buffer) {
        printf("Error allocate buffor");
        exit(1);
    }
    
    for (int i = 0; i < count; i++) {
        if (fread(buffer, sizeof(char), length, source) != length) {
            printf("Error reading file");
            exit(1);
        }
        fwrite(buffer, sizeof(char), length, target);
        if (ferror(target)) {
            printf("Error writing file");
            exit(1);
        }
    }

    free(buffer);

    if(fclose(source) != 0 || fclose(target) != 0) {
        printf("Error closing file");
        exit(1);
    }
}

void copy_sys(char *file_name_1, char *file_name_2, int count, int length) {
    int source = open(file_name_1, O_RDONLY);
    if (!source) {
        printf("Error opening file");
        exit(1);
    }
    int target = open(file_name_2, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (!target) {
        printf("Error opening file");
        exit(1);
    }

    unsigned char *buffer = malloc(length);
    if (!buffer) {
        printf("Error allocate buffor");
        exit(1);
    }

    for (int i = 0; i < count; i++) {
        if (read(source, buffer, length) != length) {
            printf("Error reading file");
            exit(1);
        }
        if (write(target, buffer, length) == -1) {
            printf("Error writing file");
            exit(1);
        }
    }

    free(buffer);

    if(close(source) != 0 || close(target) != 0) {
        printf("Error closing file");
        exit(1);
    }
}