#ifndef PROMPT_H
#define PROMPT_H

/**
 * Displays the shell prompt in the format: <Username@SystemName:CurrentDir>
 * * @param home_dir The absolute path of the directory from which the shell was invoked.
 */
void display_prompt(char *home_dir);

#endif