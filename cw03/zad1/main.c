#define _XOPEN_SOURCE 500
#include <assert.h>
#include <dirent.h>
#include <ftw.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include <unistd.h>
#include <sys/wait.h>



void tree(const char *path, const char *rel_path);
void print_dir(const char* path, const char* rel_path);


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Not enought arguments");
        exit(1);
    }
    char* dir_name = realpath(argv[1], NULL);
    if (!dir_name) {
        printf("Could not find folder");
        exit(1);
    }

    tree(dir_name, ".");
    free(dir_name);

    return 0;
}

void tree(const char* path, const char* rel_path) {
	assert(path && rel_path);

	print_dir(path, rel_path);
	DIR* dir;
	struct dirent* ent;
	struct stat stat;

	if (!(dir = opendir(path))) {
        printf("Could not open directory");
        exit(1);
    }

	while ((ent = readdir(dir))) {
		if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
			continue;

		char* new_path = malloc(strlen(path) + strlen(ent->d_name) + 3);
		char* new_rel_path = malloc(strlen(rel_path) + strlen(ent->d_name) + 3);
		if (!new_path || !new_rel_path) {
            printf("Could not allocate memory");
            exit(1);
        }
		sprintf(new_path, "%s/%s", path, ent->d_name);
		if (strcmp(".", rel_path) == 0) {
			sprintf(new_rel_path, "%s", ent->d_name);
		} else {
			sprintf(new_rel_path, "%s/%s", rel_path, ent->d_name);
		}

		if (lstat(new_path, &stat) == -1) {
            printf("Could not load stats");
            exit(1);
        }

		if (S_ISDIR(stat.st_mode)) {
			tree(new_path, new_rel_path);
		}
		free(new_path);
		free(new_rel_path);
	}

	if (closedir(dir) == -1) {
        printf("Could not close directory");
        exit(1);
    }
}

void print_dir(const char *path, const char *rel_path) {
    assert(path);
    pid_t child_pid = fork();

    if (child_pid == 0) {
        printf("\nDIR:'%s'\nPID: %d\n", rel_path, (int) getpid());
        execlp("ls", "ls", "-l", path, NULL);
    } else if  (child_pid < 0) {
        printf("Could not open new process\n");
    } else {
        int stat;
        wait(&stat);
        if (stat) {
            printf("Child process failed\n");
        }
    }
}
