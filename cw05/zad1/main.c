#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_CMDS 15
#define MAX_ARGS 15

typedef struct {
    char *name;
    char **argv;
} cmd;

typedef struct {
    cmd *list;
    size_t size;
} cmds;

char *read_file(char*);
cmds *parse_commands(char*);
cmd *parse_command(char*);
void create_pipes(cmds*);
void spawn_children(cmds*);
char **get_args(char*);

void stop(char*);

int main(int argc, char *argv[]) {
    if (argc != 2) stop("Not correct argument set.\n");
    char *cmd_str = read_file(argv[1]);
    cmds *cmds = parse_commands(cmd_str);
    spawn_children(cmds);

    for (int i = 0; i < cmds->size; i++) wait(NULL);

    return 0;
}

char* read_file(char *file_name) {
    FILE *file = fopen(file_name, "r");
    if (!file) stop("Could not op en file.\n");
    
    if (fseek(file, 0, SEEK_END) != 0) stop("Could not set point in file.\n");
    size_t file_size = ftell(file);
    if (fseek(file, 0, SEEK_SET) != 0) stop("Could not set point in file.\n");

    char* file_buf = calloc(file_size+1, sizeof(char));
    if (!file_buf) stop("Could not allocate space.\n");
    
    size_t read_chars = fread(file_buf, 1, file_size, file);

    if (read_chars != file_size) stop("Cloud not read all chars.\n");

    if (fclose(file) != 0) stop("Could not close file.\n");
    
    return file_buf;
}

cmds *parse_commands(char *commands) {
    assert(commands);

    cmd *list = calloc(MAX_CMDS + 1, sizeof(cmd));
    if (!list) stop("Could not allocate space.\n");

    char *save_ptr;
	char *cmd_str = strtok_r(commands, "|\n", &save_ptr);

	if (cmd_str == NULL) stop("Could not separate commends.\n");

	int i;
	for (i = 0; cmd_str != NULL; i++) {
		if (i < MAX_CMDS) list[i] = *parse_command(cmd_str);
		else stop("Could not parse command.");
        cmd_str = strtok_r(NULL, "|\n", &save_ptr);
	}

    cmds *cmds = calloc(1, sizeof(cmds));
    if (!cmds) stop("Could not allocate space.\n");
    cmds->size = i;
    cmds->list = list;

    return cmds;
}

cmd *parse_command(char *command) {
    assert(command);
    
    cmd *cmd = calloc(1, sizeof(cmd));

    char **args = calloc(MAX_ARGS + 1, sizeof(char*));
	if (!args) stop("Could not allocate space.\n");

    char *save_ptr;
    char *arg = strtok_r(command, " ", &save_ptr);

	if (arg == NULL) stop("The command should not be empty.");

	for (int i = 0; arg != NULL; i++) {
		if (i == 0) cmd->name = arg;
		if (i < MAX_ARGS) args[i] = arg;
		else stop("Too many arguments in command");
        arg = strtok_r(NULL, " ", &save_ptr);
	}

    cmd->argv = args;
    return cmd;
}

void spawn_children(cmds *commands) {
	int fd1[2], fd2[2], *fd3, *fd4;
	for (int i = 0; i < commands->size; i++) {
		if(i%2 == 0) {
			fd3 = fd1;
			fd4 = fd2;
		} else {
			fd3 = fd2;
			fd4 = fd1;
		}
		if (i > 0) {
	        close(fd3[0]);
	        close(fd3[1]);
	    }
		cmd command = commands->list[i];
		if(pipe(fd3) == -1) stop("Pipe error.\n");
			pid_t child_pid = fork();
			if(child_pid == 0) {
				if (i < commands->size - 1) {
	                close(fd3[0]);
	                if(dup2(fd3[1], STDOUT_FILENO) < 0) stop("Duplicate error.\n");
	            }
	            if (i > 0) {
	                close(fd4[1]);
	                if(dup2(fd4[0], STDIN_FILENO) < 0) stop("Duplicate error.\n");
	            }
				execvp(command.name, command.argv);
			}
		}
	close(fd1[0]);
	close(fd1[1]);
	close(fd2[0]);
    close(fd2[1]);
}

void stop(char *message) {
    printf("%s", message);
    exit(1);
}