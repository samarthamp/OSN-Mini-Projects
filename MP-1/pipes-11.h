#ifndef PIPES_11_H
#define PIPES_11_H

/**
 * Handles the pipeline logic (Spec 11 & 12).
 * Splits input by '|', creates pipes, handles redirection (Spec 10) for each segment,
 * and executes commands.
 * * @param input The command line string (e.g., "cat < a.txt | wc").
 */
void execute_pipeline(char *input, int is_bg, char *home_dir, char *prev_dir);

#endif