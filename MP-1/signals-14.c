#include "header.h"
#include "signals-14.h"
#include "activities-13.h"

// Global Definitions
pid_t current_fg_pid = -1;
char current_fg_name[1024] = "";

void execute_ping(char **args) {
    if (args[1] == NULL || args[2] == NULL) {
        printf("ping: missing arguments\n");
        return;
    }

    pid_t pid = atoi(args[1]);
    int sig_num = atoi(args[2]);

    // Modulo 32 as per spec
    sig_num = sig_num % 32;

    if (kill(pid, 0) == -1) {
        printf("No such process found\n");
        return;
    }

    if (kill(pid, sig_num) == 0) {
        printf("Sent signal %d to process with pid %d\n", sig_num, pid);
    } else {
        perror("ping");
    }
}

void handle_sigint(int sig) {
    if (current_fg_pid != -1) {
        // Pass signal to the foreground child
        kill(current_fg_pid, SIGINT);
    } else {
        printf("\n");
    }
}

void handle_sigtstp(int sig) {
    if (current_fg_pid != -1) {
        kill(current_fg_pid, SIGTSTP);
    } else {
        // Do nothing if no foreground process
        printf("\n");
    }
}