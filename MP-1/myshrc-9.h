#ifndef MYSHRC_9_H
#define MYSHRC_9_H

/**
 * Loads and parses the .myshrc file from the home directory.
 * Populates internal alias and function tables.
 * @param home_dir Absolute path to home.
 */
void load_myshrc(char *home_dir);

/**
 * Checks if the given command name is an alias.
 * @param cmd_name The first word of the user input.
 * @return The expanded string if found, NULL otherwise.
 */
char* get_alias(char *cmd_name);

/**
 * Checks if the given command name is a function and executes it.
 * @param cmd_name The function name.
 * @param arg The argument passed to the function ($1).
 * @param home_dir Context for execution.
 * @param prev_dir Context for execution.
 * @return 1 if executed, 0 if not found.
 */
int execute_myshrc_function(char *cmd_name, char *arg, char *home_dir, char *prev_dir);

#endif