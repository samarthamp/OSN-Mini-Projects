#ifndef PROCLORE_7_H
#define PROCLORE_7_H

/**
 * Executes the 'proclore' command.
 * Gets process info from /proc/[pid]/stat and /proc/[pid]/exe.
 * * @param args Arguments array (args[1] is optional PID).
 * @param home_dir Shell's home directory (for path formatting).
 */
void execute_proclore(char **args, char *home_dir);

#endif