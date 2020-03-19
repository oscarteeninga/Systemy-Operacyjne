#define _XOPEN_SOURCE 500
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define TIME_FMT "_%Y-%m-%d_%H-%M-%S"

char *read_lines(char *file_name);
int monitor(char *file_name, int interval, int time, int mode);
void watch_1(char *file_name, int interval, int time);
void watch_2(char *file_name, int interval, int time);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Not enought arguments\n");
        exit(1);
    }

    char *file_list = argv[1];
    int monitoring_time = atoi(argv[2]);
    int mood = atoi(argv[3]);

    if (mood != 1 && mood != 2) {
        printf("Unkonwn mode\n");
        exit(1);
    }

    char *list_buffer = read_lines(file_list);

    char *file_name = calloc(255, sizeof(char));
    if (!file_name) {
        printf("Could not allocate\n");
        exit(1);
    }

    int interval;
    char *lines = list_buffer;
    int child_count = 0;

    while (lines) {
        char *curr_line = strchr(lines, '\n');
        if (curr_line) curr_line[0] = '\0';

        if (sscanf(lines, "%255s %d", file_name, &interval) == 2) {
            child_count += monitor(file_name, interval, monitoring_time, mood);
        } else {
            fprintf(stderr, "Line '%s' ignored\n", lines);
        }

        if (curr_line) curr_line[0] = '\n';

        lines = curr_line && strcmp(curr_line, "\n") != 0 
            ? &curr_line[1] 
            : NULL;
    }

    free(list_buffer);
    free(file_name);

    for (int i = 0; i < child_count; i++) {
        int status;
        pid_t child_pid = wait(&status);
        if (WIFEXITED(status)) {
            printf("Process %d has made %d backups\n", (int) child_pid, WEXITSTATUS(status));
        } else {
            printf("Process %d has been terminated anormally\n", (int) child_pid);
        }
    }

    return 0;
}

char *read_lines(char *file_name) {
    struct stat stat;
    if (lstat(file_name, &stat) != 0) {
        printf("Could not load stats\n");
        exit(1);
    }

    FILE *file = fopen(file_name, "r");
    if (!file) {
        printf("Cloud not open file\n");
        return NULL;
    }

    char *file_buffer = calloc(stat.st_size + 1, sizeof(char));

    if (!file_buffer) {
        printf("Could not allocate space\n");
        fclose(file);
        return NULL;
    }
    if (fread(file_buffer, 1, stat.st_size, file) != stat.st_size) {
        printf("Could not read file\n");
        fclose(file);
        return NULL;
    }

    file_buffer[stat.st_size] = '\0';

    fclose(file);
    
    return file_buffer;
}

int monitor(char *file_name, int interval, int time, int mode) {
    pid_t child_pid = fork();

    if (child_pid == 0) {
        if (mode == 1) {
            watch_1(file_name, interval, time);
        }
        else {
            watch_2(file_name, interval, time);
        } 
    } else if (child_pid == -1) {
            fprintf(stderr, "Failed to start process for: %s\n", file_name);
            return 0;
    }
    return 1;
}

void watch_1 (char *file_name, int interval, int time) {
    int elapsed = 0;
    int n = 0;
    struct stat stat;

    if (lstat(file_name, &stat) == -1) {
        fprintf(stderr, "Failed to load file stats for '%s'\n", file_name);
        exit(0);
    }

    time_t last_mod = stat.st_mtime;

    char *contents = read_lines(file_name);
    if (!contents) {
        fprintf(stderr, "Error with reading file\n");
        exit(n);
    }

    char *file_name_date = calloc(strlen(file_name) + 30, sizeof(char));

    strcpy(file_name_date, file_name);

    while ((elapsed += interval) <= time) {
        sleep(interval);

        if (lstat(file_name, &stat) == -1) {
            fprintf(stderr,"Failed to load file stats for '%s'\n", file_name);
            break;
        }

        if (stat.st_mtime > last_mod) {
            strftime(&file_name_date[strlen(file_name)], 30, TIME_FMT, gmtime(&last_mod));

            FILE *backup = fopen(file_name_date, "w");

            fwrite(contents, 1, strlen(contents), backup);

            fclose(backup);

            free(contents);

            contents = read_lines(file_name);
            if (!contents) {
                printf("Error with reading contents\n");
                exit(1);
            }

            last_mod = stat.st_mtime;
            n++;
        }
    }

    free(contents);
    free(file_name_date);

    exit(n);
}

void watch_2 (char *file_name, int interval, int time) {
    int elapsed = 0;
    int n = 0;
    struct stat stat;
    
    if (lstat(file_name, &stat) == -1) {
        fprintf(stderr,"Failed to load file stats for '%s'\n", file_name);
        exit(0);
    }

    time_t last_mod = stat.st_mtime;

    char *file_name_date = calloc(strlen(file_name) + 30, sizeof(char));

    strcpy(file_name_date, file_name);

    while ((elapsed += interval) <= time) {
        if (lstat(file_name, &stat) == -1) {
            fprintf(stderr,"Failed to load file stats for '%s'\n", file_name);
            break;
        }

        strftime(&file_name_date[strlen(file_name)], 30, TIME_FMT, gmtime(&last_mod));

        if (stat.st_mtime > last_mod || n == 0) {
            last_mod = stat.st_mtime;
            pid_t child_pid = vfork();

            if (child_pid == 0) {
                if(execlp("cp", "cp", file_name, file_name_date, NULL) == -1) {
                    fprintf(stderr, "Could not exec cp\n");
                }
            } else if (child_pid == -1) {
                fprintf(stderr, "Could not backup '%s'\n", file_name);
            } else {
                int status;
                wait(&status);
                if (status != 0) {
                    fprintf(stderr, "Could not backup '%s'\n", file_name);
                }
                else {
                    n++;
                }
            }
        }
        sleep(interval);
    }
    free(file_name_date);

    exit(n);
}