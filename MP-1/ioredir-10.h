#ifndef IOREDIR_10_H
#define IOREDIR_10_H

/**
 * Parses the command string for redirection symbols (<, >, >>).
 * Opens necessary files and uses dup2 to redirect STDIN/STDOUT.
 * Modifies the input string to remove the redirection parts.
 * * @param cmd The command string to parse.
 * @return 0 on success, -1 on failure (e.g., input file not found).
 */
int handle_redirection(char *cmd);

/**
 * Restores STDIN and STDOUT to their original state.
 * Used for built-in commands running in the parent process.
 */
void restore_io(int saved_stdin, int saved_stdout);

#endif