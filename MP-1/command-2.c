#include "header.h"
#include "command-2.h"
#include "hop-3.h"
#include "reveal-4.h"
#include "log-5.h"
#include "proclore-7.h"
#include "seek-8.h"
#include "myshrc-9.h"
#include "pipes-11.h"
#include "activities-13.h"
#include "signals-14.h"
#include "fgbg-15.h"
#include "neonate-16.h"
#include "iman-17.h"

/* Define exactly once */
bg_job bg_jobs[MAX_BG];
int bg_count = 0;

void strip_quotes(char *s) {
    int len = strlen(s);
    if (len >= 2 && 
       ((s[0] == '"' && s[len-1] == '"') ||
        (s[0] == '\'' && s[len-1] == '\''))) {
        memmove(s, s + 1, len - 2);
        s[len - 2] = '\0';
    }
}

void execute_single_command(char *cmd_str, int is_bg, char *home_dir, char *prev_dir) {
    // 1. Pre-parsing Check for Alias/Functions
    // We need to look at the first word without modifying cmd_str destructively yet.
    char temp_cmd[1024];
    strcpy(temp_cmd, cmd_str);
    char *first_word = strtok(temp_cmd, " \t\n");
    
    if (first_word == NULL) return;
    // --- CHECK ALIAS ---
    char *alias_val = get_alias(first_word);

    if (alias_val != NULL) {
        // Reconstruct command: alias_value + rest of args
        char new_cmd[1024];
        strcpy(new_cmd, alias_val);
        
        // Find end of first word in ORIGINAL string to get arguments
        char *args_ptr = cmd_str;
        while (*args_ptr == ' ' || *args_ptr == '\t') args_ptr++; // skip lead
        while (*args_ptr != ' ' && *args_ptr != '\t' && *args_ptr != '\0') args_ptr++; // skip word
        
        // Append arguments
        strcat(new_cmd, args_ptr);
        
        // RECURSE with new command string (allows alias -> built-in)
        execute_single_command(new_cmd, is_bg, home_dir, prev_dir);
        return;
    }

    // --- CHECK FUNCTION ---
    // Extract first argument for function (simple implementation supports $1)
    // We reuse temp_cmd tokenization
    char *func_arg = strtok(NULL, " \t\n"); // The second token
    
    if (execute_myshrc_function(first_word, func_arg, home_dir, prev_dir)) {
        return; // Function executed
    }

    // --- NORMAL EXECUTION (If not alias or function) ---

    char *argv[64];
    int argc = 0;

    char *p = cmd_str;

    while (*p) {
        while (*p == ' ' || *p == '\t')
            p++;  // skip whitespace

        if (*p == '\0')
            break;

        if (*p == '"' || *p == '\'') {
            char quote = *p++;
            argv[argc++] = p;

            while (*p && *p != quote)
                p++;

            if (*p)
                *p++ = '\0';  // terminate token
        } else {
            argv[argc++] = p;

            while (*p && *p != ' ' && *p != '\t')
                p++;

            if (*p)
                *p++ = '\0';
        }
    }

    argv[argc] = NULL;

    if (argc == 0) return;

    // Strip Quotes from arguments
    for (int i = 0; i < argc; i++) {
        strip_quotes(argv[i]);
    }

    // --- BUILT-INS ---
    if (strcmp(argv[0], "hop") == 0) {
        execute_hop(argv, home_dir, prev_dir);
        return;
    }
    if (strcmp(argv[0], "reveal") == 0) {
        execute_reveal(argv, home_dir, prev_dir);
        return;
    }
    // Log command
    if (strcmp(argv[0], "log") == 0) {
        execute_log(argv, home_dir, prev_dir);
        return;
    }

    // Proclore command
    if (strcmp(argv[0], "proclore") == 0) {
        execute_proclore(argv, home_dir);
        return;
    }

    // Seek command
    if (strcmp(argv[0], "seek") == 0) {
        execute_seek(argv, home_dir, prev_dir);
        return;
    }

    // Activities
    if (strcmp(argv[0], "activities") == 0) { 
        execute_activities(); 
        return; 
    }

    // Ping
    if (strcmp(argv[0], "ping") == 0) {
        execute_ping(argv);
        return;
    }

    // fg
    if (strcmp(argv[0], "fg") == 0) { 
        execute_fg(argv); 
        return; 
    }

    // bg
    if (strcmp(argv[0], "bg") == 0) { 
        execute_bg(argv); 
        return; 
    }

    // neonate
    if (strcmp(argv[0], "neonate") == 0) {
        execute_neonate(argv);
        return;
    }

    // iman
    if (strcmp(argv[0], "iman") == 0) {
        execute_iman(argv);
        return;
    }

    // --- EXTERNAL ---
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return;
    }
    if (pid == 0) {
        // Child: Reset signals
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        
        if (is_bg) setpgid(0, 0);
        
        if (execvp(argv[0], argv) == -1) {
            printf("ERROR: '%s' is not a valid command\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    } else {
        if (is_bg) {
            bg_jobs[bg_count].pid = pid;
            strcpy(bg_jobs[bg_count].name, argv[0]); // command name
            bg_count++;
            printf("[%d] %d\n", 1, pid);
            add_process(pid, argv[0]);
        } else {
            // SET GLOBAL FOREGROUND PID
            current_fg_pid = pid;
            strcpy(current_fg_name, argv[0]);

            // If a child is stopped via Ctrl-Z, waitpid returns.
            // We need to check HOW it returned.
            int status;
            waitpid(pid, &status, WUNTRACED);
            
            // Check if child stopped (Ctrl+Z)
            if (WIFSTOPPED(status)) {
                printf("\n[%d] Stopped %s\n", pid, argv[0]);
                add_process(pid, argv[0]); // Add to activities list
            }            

            current_fg_pid = -1;
        }
    }
}

void process_input(char *input, char *home_dir, char *prev_dir) {
    // Standard parsing loop (handles ; and &)
    char *ptr = input;
    char *start = input;
    
    while (*ptr != '\0') {
        if (*ptr == ';' || *ptr == '&') {
            int is_bg = (*ptr == '&');
            *ptr = '\0';
            execute_pipeline(start, is_bg, home_dir, prev_dir);
            start = ptr + 1;
        }
        ptr++;
    }
    
    if (*start != '\0') {
        execute_pipeline(start, 0, home_dir, prev_dir);
    }
}