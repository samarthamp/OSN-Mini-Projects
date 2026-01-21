#ifndef LOG_5_H
#define LOG_5_H

/**
 * Adds a raw command string to the history file.
 * Handles: 
 * - Max 15 lines (FIFO)
 * - Duplicate checks (most recent only)
 * - "log" string filtering
 * * @param command The raw input string.
 * @param home_dir Absolute path to home (location of history file).
 */
void add_to_log(char *command, char *home_dir);

/**
 * Executes the 'log' command functionality.
 * Supports: no-args (print), "purge", "execute <index>".
 */
void execute_log(char **args, char *home_dir, char *prev_dir);

#endif