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
#include <signal.h>

#define TIME_FMT "_%Y-%m-%d_%H-%M-%S"

typedef struct {
    char *file_name;
    pid_t pid;
    int interval;
    int stopped;
} child;

typedef struct {
    child *list;
    int count;
} children;

//Methods for parents
void register_sig_handler();
static void sig_parent_handler(int signo);
void list(children *children);
child *find_child(children *children, pid_t child_pid);

void stop_pid(children *children, pid_t child_pid);
void stop(child *child);
void stop_all(children *children);

void start_pid(children *children, pid_t child_pid);
void start(child *child);
void start_all(children *children);

void end(children *children);

//Methods for spawining children
pid_t monitor(char *file_name, int interval);
void watch(char *file_name, int interval);
void raport(children *children);

//Varables/Methods for children
int stopped = 0;
int finish = 0;
static void sig_child_handler(int signo);

//Performance methods
char *read_lines(char *file_name);
int lines(char *str);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Not enought arguments!\n");
        exit(1);
    }

    register_sig_handler();

    char *file_list = argv[1];
    char *list_buffer = read_lines(file_list);

    if (!list_buffer) {
        printf("Cannot read file!\n");
        exit(1);
    }

    int expected_child_count = lines(list_buffer);
    child *children_list = calloc(expected_child_count, sizeof(child));

    char *file_name = calloc(255, sizeof(char));
    if (!file_name) {
        printf("Could not allocate!\n");
        exit(1);
    }

    int interval;
    char *lines = list_buffer;
    int child_count = 0;

    while (lines) {
        char *curr_line = strchr(lines, '\n');
        if (curr_line) curr_line[0] = '\0';

        if (sscanf(lines, "%255s %d", file_name, &interval) == 2) {
            pid_t child_pid = monitor(file_name, interval);
            if (child_pid) {
                child *child = &children_list[child_count];
                child->file_name = calloc(strlen(file_name) + 1, sizeof(char));
                if (!child->file_name) {
                    printf("Cannot allocate!\n");
                    exit(1);
                }
                strcpy(child->file_name, file_name);
                child->pid = child_pid;
                child->interval = interval;
                child_count++;
            }
        } else {
            printf("Line '%s' ignored!\n", lines);
        }

        if (curr_line) curr_line[0] = '\n';

        lines = curr_line && strcmp(curr_line, "\n") != 0 
            ? &curr_line[1] 
            : NULL;
    }

    children_list = realloc(children_list, child_count * sizeof(child));

    free(list_buffer);
    free(file_name);

    children children;
    children.list = children_list;
    children.count = child_count;

    list(&children);

    char command[15];

    while (!finish) {
        fgets(command, 15, stdin);

        if (strcmp(command, "LIST\n") == 0) {
            list(&children);
        } else if (strcmp(command, "END\n") == 0) {
            break;
        } else if (strcmp(command, "STOP ALL\n") == 0) {
            stop_all(&children);
        } else if (strncmp(command, "STOP ", 5) == 0) {
            int pid = atoi(command+5);
            if (pid > 0) stop_pid(&children, pid);
            else printf("Invaild PID: %d\n", pid);
        } else if (strcmp(command, "START ALL\n") == 0) {
            start_all(&children);
        } else if (strncmp(command, "START ", 6) == 0) {
            int pid = atoi(command+6);
            if (pid > 0) start_pid(&children, pid);
            else printf("Invaild PID: %d", pid);
        } else {
            printf("Unknown command.\n");
        }
    }

    for (int i = 0; i < child_count; i++) {
        int status;
        kill(children.list[i].pid, SIGUSR2);
        pid_t child_pid = waitpid(children.list[i].pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("Process %d has made %d backups!\n", (int) child_pid, WEXITSTATUS(status));
        } else {
            printf("Process %d has been terminated anormally!\n", (int) child_pid);
        }
    }

    for (int i = 0; i < child_count; i++) {
        free(children.list[i].file_name);
    }

    free(children.list);

    return 0;
}

void register_sig_handler() {
    struct sigaction action;
    action.sa_handler = sig_parent_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGINT, &action, NULL)) {
        printf("Cannot define SIGINT action!\n");
        exit(1);
    }
}

static void sig_parent_handler(int signo) {
    if (signo == SIGINT) finish = 1;
}

void list(children *children) {
    assert(children);

    for (int i = 0; i < children->count; i++) {
        printf("Process %d in monitoring %s every %d seconds.\n", children->list[i].pid, children->list[i].file_name, children->list[i].interval);
    }
}

child *find_child(children *children, pid_t child_pid) {
    assert(children);

    for (int i = 0; i < children->count; i++) {
        if (children->list[i].pid == child_pid) {
            return &children->list[i];
        }
    }
    printf("Child with PID: %d does not exist!\n", child_pid);
    return NULL;
}

void stop_pid(children *children, pid_t child_pid) {
    child *child = find_child(children, child_pid);
    if (child) stop(child);
}

void stop(child *child) {
    assert(child);

    if (child->stopped) {
        printf("Child with PID: %d was already stopped.\n", child->pid);
        return;
    }

    if (kill(child->pid, SIGUSR1) == -1) {
        printf("Cannot send signal to child with PID: %d.\n", child->pid);
        exit(1);
    } else {
        printf("Child with PID: %d stopped.\n", child->pid);
        child->stopped = 1;
    }
}

void stop_all(children *children) {
    assert(children);

    for (int i = 0; i < children->count; i++) {
        stop(&children->list[i]);
    }
}

void start_pid(children *children, pid_t child_pid) {
    child *child = find_child(children, child_pid);
    if (child) start(child);
}

void start(child *child) {
    assert (child);

    if (!child->stopped) {
        printf("Child with PID: %d is already running.\n", child->pid);
        return;
    }

    if (kill(child->pid, SIGUSR1) == -1) {
        printf("Cannot send signal to child with PID: %d.\n", child->pid);
    } else {
        printf("Child with PID: %d resterted.\n", child->pid);
        child->stopped = 0;
    }
}

void start_all(children *children) {
    assert(children);

    for (int i = 0; i < children->count; i++) {
        start(&children->list[i]);
    }
}

pid_t monitor(char *file_name, int interval) {
    pid_t child_pid = fork();

    if (child_pid == 0) {
        struct sigaction action;
        action.sa_handler = sig_child_handler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;

        if (sigaction(SIGUSR1, &action, NULL) || sigaction(SIGUSR2, &action, NULL)) {
            printf("Cannot define handling for SIGUR signal");
            exit(1);
        }

        watch(file_name, interval);
    } else if (child_pid == -1) {
            printf("Failed to start process for: %s\n", file_name);
            return 0;
    }
    return child_pid;
}

void watch (char *file_name, int interval) {
    int n = 0;
    struct stat stat;

    if (lstat(file_name, &stat) == -1) {
        printf("Failed to load file stats for '%s'.\n", file_name);
        exit(0);
    }

    time_t last_mod = stat.st_mtime;

    char *contents = read_lines(file_name);
    if (!contents) {
        printf("Error with reading file.\n");
        exit(n);
    }

    char *file_name_date = calloc(strlen(file_name) + 30, sizeof(char));
    strcpy(file_name_date, file_name);

    while (1) {
        sleep(interval);

        if (finish) break;
        if (stopped) continue;

        if (lstat(file_name, &stat) == -1) {
            printf("Failed to load file stats for '%s'\n", file_name);
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
                exit(n);
            }

            last_mod = stat.st_mtime;
            n++;
        }
    }

    free(contents);
    free(file_name_date);

    exit(n);
}

static void sig_child_handler(int signo) {
    if (signo == SIGUSR1) {
        stopped = !stopped;
    } else {
        finish = 1;
    }
}

char *read_lines(char *file_name) {
	assert(file_name);

	struct stat stat;
	if (lstat(file_name, &stat) != 0) {
        printf("Cannot load stats of file!\n");
        exit(1);
    }

	FILE* file = fopen(file_name, "r");
	if (!file) return NULL;

	char *filebuffer = malloc(stat.st_size + 1);
	if (!filebuffer || fread(filebuffer, 1, stat.st_size, file) != stat.st_size) {
		free(filebuffer);
		fclose(file);
		return NULL;
	}

	filebuffer[stat.st_size] = '\0';

	fclose(file);

	return filebuffer;
}

int lines(char *str) {
    assert(str);

    int count = 0;
    char *i = str;
    while (i) {
        i = strchr(i, '\n');
        if (i) {
            i++;
            count++;
        }
    }
    return count;
}