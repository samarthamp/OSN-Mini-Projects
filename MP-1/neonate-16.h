#ifndef NEONATE_16_H
#define NEONATE_16_H

/**
 * Executes the neonate command.
 * Prints the most recent PID every N seconds until 'x' is pressed.
 * @param args Arguments array (expects: neonate -n [time]).
 */
void execute_neonate(char **args);

#endif