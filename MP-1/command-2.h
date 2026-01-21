#ifndef COMMAND_2_H
#define COMMAND_2_H

/**
 * Entry point for processing a raw input line (handling ; and &).
 * Delegates actual execution to pipes-11.c.
 */
void process_input(char *input, char *home_dir, char *prev_dir);

/**
 * Parses args and executes a single command (built-in or external).
 * DOES NOT handle pipes or redirection (assumed handled by caller).
 * Handles forking ONLY for external commands. Built-ins run in current process.
 */
void execute_single_command(char *cmd_str, int is_bg, char *home_dir, char *prev_dir);

#endif