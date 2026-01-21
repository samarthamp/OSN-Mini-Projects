#ifndef REVEAL_H
#define REVEAL_H

/**
 * Lists files in a directory with specific formatting.
 * @param args Argument array (flags and path).
 * @param home_dir Absolute path to home.
 * @param prev_dir Absolute path to the previous directory (for '-').
 */
void execute_reveal(char **args, char *home_dir, char *prev_dir);

#endif