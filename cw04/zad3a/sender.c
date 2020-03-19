#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

//#define SIGRTMIN 13
//#define SIGRTMAX 14

int expected_sig_count = 0;
int sig_count = 0;

//Modes
static void kill_catch_usr(int sig, siginfo_t *info, void *context);
static void queue_catch_usr(int sig, siginfo_t *info, void *context);
static void catch_rt(int sig, siginfo_t *info, void *context);

//Signals handling
void init_signal(void(*fun) (int, siginfo_t*, void*));
void init_rt_signal(void(*fun) (int, siginfo_t*, void*));

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Not enought arguments!\n");
        exit(1);
    }

    pid_t catcher_pid = atoi(argv[1]);
    expected_sig_count = atoi(argv[2]);
    char *mode = argv[3];

    if (strcmp(mode, "KILL") == 0) {
        init_signal(kill_catch_usr);
        for (int i = 0; i < expected_sig_count; i++) {
            if (kill(catcher_pid, SIGUSR1) != 0) {
                printf("Could not send signal.\n");
                exit(1);
            }
        }
        if (kill(catcher_pid, SIGUSR2) != 0) {
            printf("Could not send signal.\n");
            exit(1);
        }
    } else if (strcmp(mode, "SIGQUEUE") == 0) {
        init_signal(queue_catch_usr);
        for (int i = 0; i < expected_sig_count; i++) {
            union sigval val = {i};
            if (sigqueue(catcher_pid, SIGUSR1, val) != 0) {
                printf("Could not send signal.\n");
                exit(1);
            }
        }
        if (kill(catcher_pid, SIGUSR2) != 0) {
            printf("Could not send signal.\n");
            exit(1);
        }
    } else if (strcmp(mode, "SIGRT") == 0) {
        init_rt_signal(catch_rt);
        for (int i = 0; i < expected_sig_count; i++) {
            if (kill(catcher_pid, SIGRTMIN) != 0) {
                printf("Could not send signal.\n");
                exit(1);    
            }
        }
        if (kill(catcher_pid, SIGRTMAX) != 0) {
            printf("Could not send signal.\n");
            exit(1);
        }
    } else {
        printf("Unknown mode.\n");
        exit(1);
    }

    while(1) pause();

    return 0;
}

static void kill_catch_usr(int sig, siginfo_t *info, void *context) {
	if (sig == SIGUSR1) {
		sig_count++;
    } else {
		printf("Expected: %d, Actual: %d\n", expected_sig_count, sig_count);
		exit(0);
	}
}

static void queue_catch_usr(int sig, siginfo_t *info, void *context) {
    if (sig == SIGUSR1) {
        sig_count++;
    } else {
        printf("Expected: %d, Actual: %d.\n", expected_sig_count, sig_count);
        exit(0);
    }
}

static void catch_rt(int sig, siginfo_t *info, void *context) {
    if (sig == SIGRTMIN) {
        sig_count++;
    } else {
        printf("Expected: %d, Actual: %d.\n", expected_sig_count, sig_count);
        exit(0);
    }
}

void init_signal(void(*fun) (int, siginfo_t*, void*)) {
	sigset_t signals;

	if (sigfillset(&signals) == -1) {
        printf("Unsucessfull initialization filled set.\n");
        exit(1);
    }

	sigdelset(&signals, SIGUSR1);
	sigdelset(&signals, SIGUSR2);

	if (sigprocmask(SIG_BLOCK, &signals, NULL) != 0) {
        printf("Unsucessfull define mask.\n");
        exit(1);
    }

	if (sigemptyset(&signals) == -1) {
        printf("Unsucessfull define mask.\n");
        exit(1);
    }

	struct sigaction usr_action;
	usr_action.sa_sigaction = fun;
	usr_action.sa_mask = signals;
	usr_action.sa_flags = SA_SIGINFO;

	if (sigaction(SIGUSR1, &usr_action, NULL) == -1 || sigaction(SIGUSR2, &usr_action, NULL) == -1) {
        printf("Unsucessfull define action.\n");
        exit(1);
    }
}

void init_rt_signal(void(*fun) (int, siginfo_t*, void*)) {
    sigset_t signals;
    if (sigfillset(&signals) == -1) {
        printf("Unsucessfull initialization filled set.\n");
        exit(1);
    }

    sigdelset(&signals, SIGRTMIN);
    sigdelset(&signals, SIGRTMAX);

    if (sigprocmask(SIG_BLOCK, &signals, NULL) != 0) {
        printf("Unsucessfull define mask.\n");
        exit(1);
    }

    if (sigemptyset(&signals) == -1) {
        printf("Unsucessfull initialization empty set.\n");
        exit(1);
    }

    struct sigaction usrs_action;
    usrs_action.sa_sigaction = fun;
    usrs_action.sa_mask = signals;
    usrs_action.sa_flags = SA_SIGINFO;

    if (sigaction(SIGRTMIN, &usrs_action, NULL) == -1 || sigaction(SIGRTMAX, &usrs_action, NULL) == -1) {
        printf("Unsucessfull define action.\n");
        exit(1);
    }
}