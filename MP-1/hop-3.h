#ifndef HOP_H
#define HOP_H

/**
 * Executes the hop command.
 * @param prev_dir Buffer holding the previous directory path (managed by main).
 */
void execute_hop(char **args, char *home_dir, char *prev_dir);

#endif