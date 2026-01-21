#ifndef SEEK_8_H
#define SEEK_8_H

/**
 * Executes the seek command.
 * Searches for files/directories matching a target name/prefix.
 * Handles flags -d, -f, -e.
 * * @param args Argument array.
 * @param home_dir Absolute path to home (for resolving ~).
 * @param prev_dir Previous directory (for resolving -).
 */
void execute_seek(char **args, char *home_dir, char *prev_dir);

#endif