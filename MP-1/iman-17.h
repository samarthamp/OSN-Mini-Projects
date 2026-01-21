#ifndef IMAN_17_H
#define IMAN_17_H

/**
 * Fetches man pages from http://man.he.net/ using sockets.
 * Prints the body of the HTTP response (stripping headers).
 * @param args Arguments array. args[1] is the command to look up.
 */
void execute_iman(char **args);

#endif