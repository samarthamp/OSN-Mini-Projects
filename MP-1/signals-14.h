#ifndef SIGNALS_14_H
#define SIGNALS_14_H

#include <sys/types.h>

/**
 * Global variable to track the currently running foreground process.
 * -1 implies no foreground process (shell is waiting for input).
 */
extern pid_t current_fg_pid;
extern char current_fg_name[1024]; // To store name for Ctrl+Z addition

/**
 * Executes the 'ping' command.
 * Sends a signal to a specific PID.
 */
void execute_ping(char **args);

/**
 * Signal Handler for SIGINT (Ctrl+C).
 */
void handle_sigint(int sig);

/**
 * Signal Handler for SIGTSTP (Ctrl+Z).
 */
void handle_sigtstp(int sig);

#endif