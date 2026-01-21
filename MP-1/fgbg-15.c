#include "header.h"
#include "fgbg-15.h"
#include "activities-13.h"
#include "signals-14.h" // For current_fg_pid global

void execute_fg(char **args) {
    if (args[1] == NULL) {
        printf("fg: missing pid\n");
        return;
    }

    pid_t pid = atoi(args[1]);
    char command_name[1024];

    // 1. Check if process exists in our list
    if (get_process_command(pid, command_name) == 0) {
        printf("No such process found\n");
        return;
    }

    // 2. Remove from background list (it is now foreground)
    remove_process(pid);

    // 3. Send SIGCONT (in case it was stopped)
    // Ignore error if it was already running
    kill(pid, SIGCONT);

    // 4. Update Global State
    current_fg_pid = pid;
    strcpy(current_fg_name, command_name);

    // 5. Wait for it (Block shell)
    // We reuse the WUNTRACED logic to handle Ctrl+Z again
    int status;    
    waitpid(pid, &status, WUNTRACED);
    
    // 6. Check if stopped again
    if (WIFSTOPPED(status)) {
        printf("\n[%d] Stopped %s\n", pid, command_name);
        add_process(pid, command_name); // Add back to list
    }

    // Reset global
    current_fg_pid = -1;
}

void execute_bg(char **args) {
    if (args[1] == NULL) {
        printf("bg: missing pid\n");
        return;
    }

    pid_t pid = atoi(args[1]);
    char temp[1024];

    // 1. Check existence
    if (get_process_command(pid, temp) == 0) {
        printf("No such process found\n");
        return;
    }

    // 2. Send SIGCONT
    if (kill(pid, SIGCONT) == 0) {
        printf("Resumed [%d] %s\n", pid, temp);
    } else {
        perror("bg");
    }
}