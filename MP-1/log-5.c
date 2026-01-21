#include "header.h"
#include "log-5.h"
#include "command-2.h" // For process_input recursion

#define MAX_LOG_SIZE 15
#define HISTORY_FILE ".myshell_history"

// Helper: Construct file path
void get_log_path(char *dest, char *home_dir) {
    snprintf(dest, PATH_MAX, "%s/%s", home_dir, HISTORY_FILE);
}

// Helper: Read all lines from file into memory
// Returns number of lines read
int read_log(char *home_dir, char history[][PATH_MAX]) {
    char path[PATH_MAX];
    get_log_path(path, home_dir);

    FILE *fp = fopen(path, "r");
    if (!fp) return 0;

    int count = 0;
    char buffer[PATH_MAX];
    // Read lines until EOF or MAX reached
    while (fgets(buffer, sizeof(buffer), fp) && count < MAX_LOG_SIZE) {
        // Strip newline
        buffer[strcspn(buffer, "\n")] = 0;
        if (strlen(buffer) > 0) {
            strcpy(history[count++], buffer);
        }
    }
    fclose(fp);
    return count;
}

// Helper: Write memory back to file
void write_log(char *home_dir, char history[][PATH_MAX], int count) {
    char path[PATH_MAX];
    get_log_path(path, home_dir);

    FILE *fp = fopen(path, "w");
    if (!fp) {
        perror("log: write failed");
        return;
    }
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s\n", history[i]);
    }
    fclose(fp);
}

void add_to_log(char *command, char *home_dir) {
    // 1. Filter: If command contains "log", do NOT store it.
    // This covers "log purge", "log execute", and "cmd ; log" cases.
    // (strstr returns a pointer if found, NULL otherwise)
    if (strstr(command, "log") != NULL) {
        return;
    }

    // 2. Clean input
    char clean_cmd[PATH_MAX];
    strncpy(clean_cmd, command, PATH_MAX - 1);
    clean_cmd[PATH_MAX - 1] = '\0';
    clean_cmd[strcspn(clean_cmd, "\n")] = 0;

    if (strlen(clean_cmd) == 0) return;

    // 3. Load History
    char history[MAX_LOG_SIZE][PATH_MAX];
    int count = read_log(home_dir, history);

    // 4. Check Duplicate (Compare with most recent, which is the last in the array)
    if (count > 0 && strcmp(history[count - 1], clean_cmd) == 0) {
        return; 
    }

    // 5. Add to List
    if (count < MAX_LOG_SIZE) {
        // Space available, append
        strcpy(history[count], clean_cmd);
        count++;
    } else {
        // Full, shift left (FIFO)
        for (int i = 1; i < MAX_LOG_SIZE; i++) {
            strcpy(history[i - 1], history[i]);
        }
        strcpy(history[MAX_LOG_SIZE - 1], clean_cmd);
    }

    // 6. Save
    write_log(home_dir, history, count);
}

void execute_log(char **args, char *home_dir, char *prev_dir) {
    char history[MAX_LOG_SIZE][PATH_MAX];
    int count = read_log(home_dir, history);

    // CASE 1: "log" (Display)
    if (args[1] == NULL) {
        // Print from oldest (index 0) to newest
        for (int i = 0; i < count; i++) {
            printf("%s\n", history[i]);
        }
        return;
    }

    // CASE 2: "log purge"
    if (strcmp(args[1], "purge") == 0) {
        char path[PATH_MAX];
        get_log_path(path, home_dir);
        FILE *fp = fopen(path, "w"); // Truncate file
        if (fp) fclose(fp);
        return;
    }

    // CASE 3: "log execute <index>"
    if (strcmp(args[1], "execute") == 0) {
        if (args[2] == NULL) {
            printf("log execute: missing index\n");
            return;
        }

        int idx_arg = atoi(args[2]);
        // Specification: "ordered from most recent to oldest"
        // Index 1 = Most Recent (history[count-1])
        // Index N = Oldest (history[count-N])
        
        if (idx_arg < 1 || idx_arg > count) {
            printf("log execute: invalid index\n");
            return;
        }

        // Calculate Array Index
        // If count=3, idx=1 (newest) -> array index 2
        // If count=3, idx=3 (oldest) -> array index 0
        int array_idx = count - idx_arg;

        char *cmd_to_run = history[array_idx];
        
        // Execute
        // We make a copy because process_input might modify the string (strtok)
        char cmd_buffer[PATH_MAX];
        strcpy(cmd_buffer, cmd_to_run);
        
        // Note: We call process_input directly. This bypasses 'add_to_log' in main.c,
        // which matches the requirement (we don't strictly need to store executed log cmds, 
        // plus 'log execute' itself contains 'log' so it's filtered anyway).
        process_input(cmd_buffer, home_dir, prev_dir);
        return;
    }

    printf("log: invalid argument '%s'\n", args[1]);
}