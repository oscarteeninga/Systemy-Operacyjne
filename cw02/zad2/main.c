#define _XOPEN_SOURCE 500
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <ftw.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define TIME_FMT "%d.%m.%Y %H:%M:%S"

time_t parse_time(char* time);
void set(char* operator, clock_t time);
int time_valid(time_t file_time, char* operator, time_t time);
void tree(char* path, char* operator, time_t time);
static int tree_nftw(const char* path, const struct stat* stat, int mode, struct FTW* ftwbuf);
void show_file(const char* path, const struct stat* stat);

char* g_operator;
clock_t g_time;

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Not enought arguments");
        exit(1);
    }
    char* dir_name = realpath(argv[1], NULL);
    char* operator = argv[2];
    time_t time = parse_time(argv[3]);
    char* mode = argv[4];

    if (!dir_name) {
        printf("Couldn\t\t find directory");
        exit(1);
    }

    if (strcmp(mode, "1") == 0) {
        tree(dir_name, operator, time);
    }
    else if (strcmp(mode, "2") == 0) {
        set(operator, time);
        nftw(dir_name, tree_nftw, 20, FTW_PHYS);
    }
    else {
        printf("Mode not found, specify 1 or 2");
        exit(1);
    }
    free(dir_name);
    return 0;
}

void set(char* operator, clock_t time) {
    g_time = time;
    g_operator = operator;
}

time_t parse_time(char* time) {
    assert(time);
    struct tm date;
    char *res = strptime(time, TIME_FMT, &date);
    if (res == NULL || *res != '\0') {
        printf("Incorrect date format");
        exit(1);
    }

    time_t result = mktime(&date);
    if (result == -1) {
        printf("Cannot convert date");
        exit(1);
    }
    return result;
}

int time_valid(time_t file_time, char* operator, time_t time) {
    if (strcmp(operator, "=") == 0) return file_time == time;
    else if (strcmp(operator, "<") == 0) return file_time < time;
    else if (strcmp(operator, ">") == 0) return file_time > time;
    else {
        printf("Unknown operator");
        exit(1);
    } 
}

void tree(char* path, char* operator, time_t time) {
	assert(path && operator);

	DIR* dir;
	struct dirent* ent;
	struct stat stat;

	if (!(dir = opendir(path))) {
        printf("Cannot find dir");
        exit(1);
    }

	while ((ent = readdir(dir))) {
		if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;

		char* new_path = malloc(strlen(path) + strlen(ent->d_name) + 2);
		if (!new_path) {
            printf("Could not allocate memory");
            exit(1);
        }
		sprintf(new_path, "%s/%s", path, ent->d_name);

		if (lstat(new_path, &stat) == -1) {
            printf("Could not load stats");
            exit(1);
        }

		if (time_valid(stat.st_mtime, operator, time))
			show_file(new_path, &stat);

		if (S_ISDIR(stat.st_mode)) {
			tree(new_path, operator, time);
		}
		free(new_path);
	}

	if (closedir(dir) == -1) {
        printf("Cannot close dir");
        exit(1);
    }
}

static int tree_nftw(const char *path, const struct stat *stat, int typeflag, struct FTW *ftwbuf) {
	if (ftwbuf->level == 0) return 0;
	if (!time_valid(stat->st_mtime, g_operator, g_time)) return 0;

	show_file(path, stat);

	return 0;
}

void show_file(const char* path, const struct stat* stat) {
	assert(path && stat);

    printf("%s\n", path);

	if (S_ISREG(stat->st_mode))
		printf("zwykły plik\t\t");
	else if (S_ISDIR(stat->st_mode))
		printf("katalog\t\t\t");
	else if (S_ISCHR(stat->st_mode))
		printf("urządzenie znakowe\t");
	else if (S_ISBLK(stat->st_mode))
		printf("urzączenie blokowe\t");
	else if (S_ISFIFO(stat->st_mode))
		printf("potok nazwany\t\t");
	else if (S_ISLNK(stat->st_mode))
		printf("link symboliczny\t");
	else
		printf("soket\t\t");

	printf("%ld\t", stat->st_size);

	char* buffer = malloc(30);
	if (!buffer) {
        printf("Could not allocate memory");
        exit(1);
    }
	strftime(buffer, 30, TIME_FMT, localtime(&stat->st_atime)); //access
	printf("%s\t", buffer);
	strftime(buffer, 30, TIME_FMT, localtime(&stat->st_mtime)); //modification
	printf("%s\n\n", buffer);
	free(buffer);
}
