#include "header.h"
#include "myshrc-9.h"
#include "command-2.h" // Needed to execute function body commands recursively

#define MAX_ALIASES 10
#define MAX_FUNCS 5
#define MAX_FUNC_LINES 10

typedef struct {
    char name[64];
    char value[256];
} Alias;

typedef struct {
    char name[64];
    char lines[MAX_FUNC_LINES][256];
    int line_count;
} Function;

static Alias alias_table[MAX_ALIASES];
static int alias_count = 0;

static Function func_table[MAX_FUNCS];
static int func_count = 0;

// Helper to trim whitespace
// void trim_whitespace(char *str) {
//     char *end;
//     while(*str == ' ') str++; // Trim leading (pointer move handled by caller usually, but here we modify content)
//     // For this simple implementation, we assume caller handles start or we memmove.
//     // Simplification: We rely on strtok in parsing.
// }

void load_myshrc(char *home_dir) {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/.myshrc", home_dir);

    FILE *fp = fopen(path, "r");
    if (!fp) {
        // Create default if not exists (Optional convenience)
        return;
    }

    char line[512];
    int in_func = 0;
    int current_func_idx = -1;

    while (fgets(line, sizeof(line), fp)) {
        // Strip newline
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0 || line[0] == '/') continue; // Skip empty or comments

        if (in_func) {
            if (strchr(line, '}')) {
                in_func = 0;
            } else {
                if (func_count < MAX_FUNCS && current_func_idx != -1) {
                    Function *f = &func_table[current_func_idx];
                    if (f->line_count < MAX_FUNC_LINES) {
                        strcpy(f->lines[f->line_count++], line);
                    }
                }
            }
            continue;
        }

        // Parse Alias: "alias name = command"
        if (strncmp(line, "alias", 5) == 0) {
            char line_copy[512];
            strcpy(line_copy, line);

            char *p = line + 6; 
            char *token = strtok(p, " ="); // Split by space or =
            
            if (token && alias_count < MAX_ALIASES) {
                strcpy(alias_table[alias_count].name, token);
                
                // Get value (remainder of line after =)
                // This is a bit tricky with strtok. Let's find '=' manually in original line.

                
                char *eq = strchr(line_copy, '=');
                if (eq) {
                    char *val = eq + 1;
                    while (*val == ' ') val++; // Skip leading space
                    strcpy(alias_table[alias_count].value, val);
                    
                    alias_count++;
                }
                else {
                    printf("Malformed alias line: %s\n", line);
                }
            }
        }
        // Parse Function: "func name()"
        else if (strncmp(line, "func", 4) == 0) {
            char *p = line + 4;
            char *name = strtok(p, " ()");
            if (name && func_count < MAX_FUNCS) {
                strcpy(func_table[func_count].name, name);
                func_table[func_count].line_count = 0;
                current_func_idx = func_count;
                func_count++;
                
                // Check if '{' is on this line
                if (strchr(line, '{')) in_func = 1;
                else {
                    // Assume next line is '{', read it and ignore
                    if(fgets(line, sizeof(line), fp)) in_func = 1;
                }
            }
        }
    }

    // Print loaded aliases and functions for debugging
    // for (int i = 0; i < alias_count; i++) {
    //     printf("Loaded Alias: %s = %s\n", alias_table[i].name, alias_table[i].value);
    // }
    // for (int i = 0; i < func_count; i++) {
    //     printf("Loaded Function: %s with %d lines\n", func_table[i].name, func_table[i].line_count);
    // }

    fclose(fp);
}

char* get_alias(char *cmd_name) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(alias_table[i].name, cmd_name) == 0) {
            return alias_table[i].value;
        }
    }
    return NULL;
}

int execute_myshrc_function(char *cmd_name, char *arg, char *home_dir, char *prev_dir) {
    for (int i = 0; i < func_count; i++) {
        if (strcmp(func_table[i].name, cmd_name) == 0) {
            // Execute lines
            Function *f = &func_table[i];
            for (int j = 0; j < f->line_count; j++) {
                char raw_line[256];
                strcpy(raw_line, f->lines[j]);
                
                // Trim leading whitespace/tabs
                char *clean_line = raw_line;
                while (*clean_line == ' ' || *clean_line == '\t') clean_line++;

                if (strlen(clean_line) == 0 || clean_line[0] == '}') continue;

                // Replace "$1" with arg
                char final_cmd[512] = "";
                char *pos = strstr(clean_line, "\"$1\"");
                if (pos) {
                    // Copy part before
                    strncat(final_cmd, clean_line, pos - clean_line);
                    // Append arg
                    if (arg) strcat(final_cmd, arg);
                    // Append part after ("$1" is 4 chars)
                    strcat(final_cmd, pos + 4);
                } else {
                    strcpy(final_cmd, clean_line);
                }

                // Recursively process this command
                // Note: process_input destroys strings with strtok, pass copy
                char temp_buf[512];
                strcpy(temp_buf, final_cmd);
                process_input(temp_buf, home_dir, prev_dir);
            }
            return 1; // Executed
        }
    }
    return 0; // Not found
}