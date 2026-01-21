#include "header.h"
#include "pipes-11.h"
#include "ioredir-10.h"
#include "command-2.h"
#include "myshrc-9.h"
#include "activities-13.h"
#include "signals-14.h"

// Helper to check if a command is a built-in (or alias to one)
int is_built_in(char *cmd) {
    // 1. Check if it's an alias first
    // We need a non-destructive copy to check the first word
    char temp[1024];
    strncpy(temp, cmd, 1023);
    temp[1023] = '\0';
    
    char *first_word = strtok(temp, " \t\n");
    if (!first_word) return 0;

    char *alias_val = get_alias(first_word);
    if (alias_val != NULL) {
        // Recursively check if the alias resolves to a built-in
        return is_built_in(alias_val);
    }

    // 2. Check Key Built-ins
    // These MUST run in parent to affect shell state (hop, seek -e)
    // or are just standard built-ins.
    if (strcmp(first_word, "hop") == 0) return 1;
    if (strcmp(first_word, "reveal") == 0) return 1;
    if (strcmp(first_word, "log") == 0) return 1;
    if (strcmp(first_word, "proclore") == 0) return 1;
    if (strcmp(first_word, "seek") == 0) return 1;
    if (strcmp(first_word, "activities") == 0) return 1;

    return 0;
}

void execute_pipeline(char *input, int is_bg, char *home_dir, char *prev_dir) {
    // 1. Split by '|'
    char *commands[64]; // Max 64 commands in a pipeline  (can be adjusted)
    int num_cmds = 0;

    char *token = strtok(input, "|");
    while (token != NULL && num_cmds < 63) {
        commands[num_cmds++] = token;
        token = strtok(NULL, "|");
    }
    commands[num_cmds] = NULL;

    if (num_cmds == 0) return;

    // 2. Execution Loop
    int prev_pipe_fd = -1; // Read end of previous pipe
    int pipe_fd[2];
    
    for (int i = 0; i < num_cmds; i++) {
        commands[i][strcspn(commands[i], "\n")] = 0; // Strip newline
    }
    
    // --- OPTIMIZATION FOR SINGLE BUILT-IN COMMAND ---
    if (num_cmds == 1 && is_built_in(commands[0])) {
        // Save IO
        int saved_stdin = dup(STDIN_FILENO);
        int saved_stdout = dup(STDOUT_FILENO);

        // Handle Redirection
        if (handle_redirection(commands[0]) == -1) {
            restore_io(saved_stdin, saved_stdout);
            return;
        }

        // Execute Built-in
        execute_single_command(commands[0], is_bg, home_dir, prev_dir);
        
        // Restore IO
        restore_io(saved_stdin, saved_stdout);
        return;
    }

    for (int i = 0; i < num_cmds; i++) {
        // Create pipe for all except the last command
        if (i < num_cmds - 1) {
            if (pipe(pipe_fd) < 0) {
                perror("pipe");
                return;
            }
        }        

        // --- PIPELINE (Forking required) ---
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            return;
        }

        if (pid == 0) {
            // Child: Reset signals to default
            // Important because parent shell ignores/catches them
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            
            if (is_bg) setpgid(0, 0);

            // 1. Wiring Input (from previous pipe)
            if (i > 0) {
                dup2(prev_pipe_fd, STDIN_FILENO);
                close(prev_pipe_fd);
            }

            // 2. Wiring Output (to current pipe)
            if (i < num_cmds - 1) {
                dup2(pipe_fd[1], STDOUT_FILENO);
                close(pipe_fd[1]);
                close(pipe_fd[0]); // Close read end in writer child
            }

            // 3. Handle Redirection (Overrides pipes)
            if (handle_redirection(commands[i]) == -1) {
                exit(EXIT_FAILURE);
            }

            // 4. Execute
            execute_single_command(commands[i], 0, home_dir, prev_dir); 
            exit(0);
        } else {
            // Parent Process
            // Close the read end of the previous pipe (we are done with it)
            if (i > 0) {
                close(prev_pipe_fd);
            }
            // Close the write end of the current pipe (child has it)
            if (i < num_cmds - 1) {
                close(pipe_fd[1]);
                // Save the read end for the next iteration
                prev_pipe_fd = pipe_fd[0];
            }

            // Wait only if foreground
            if (!is_bg) {
                // SET GLOBAL FOREGROUND PID
                current_fg_pid = pid;
                strcpy(current_fg_name, commands[i]); // Store name for Ctrl-Z

                waitpid(pid, NULL, 0);
                
                // RESET
                current_fg_pid = -1;
            } else {
                printf("[%d] %d\n", i+1, pid);
                // ADD TO PROCESS LIST
                add_process(pid, commands[i]);
            }
        }
    }
}