#include "header.h"
#include "proclore-7.h"

void execute_proclore(char **args, char *home_dir) {
    pid_t pid;

    // 1. Determine PID
    if (args[1] == NULL) {
        pid = getpid(); // Shell's own PID
    } else {
        pid = atoi(args[1]);
    }

    // 2. Open /proc/[pid]/stat
    char stat_path[PATH_MAX];
    snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);

    FILE *fp = fopen(stat_path, "r");
    if (!fp) {
        printf("proclore: Process %d does not exist or access denied\n", pid);
        return;
    }

    // 3. Parse 'stat' file
    // Format: pid (comm) state ppid pgrp session tty_nr tpgid ... vsize (field 23)
    // We need: State (3), PGRP (5), TPGID (8), VmSize (23)
    
    char state;
    int pgrp, tpgid;
    unsigned long vsize;
    
    // We scan strictly. 
    // We will use a robust skipping method for the comm field.
    
    int dummy_int;
    char dummy_str[1024]; // To hold comm
    
    // Read PID
    fscanf(fp, "%d", &dummy_int);
    
    fscanf(fp, " %s ", dummy_str); // This reads "(name)"
    
    // Read State
    fscanf(fp, " %c", &state);
    
    // Read PPID (4) -> skip
    fscanf(fp, " %d", &dummy_int);
    
    // Read PGRP (5)
    fscanf(fp, " %d", &pgrp);
    
    // Skip Session (6), TTY (7)
    fscanf(fp, " %d %d", &dummy_int, &dummy_int);
    
    // Read TPGID (8)
    fscanf(fp, " %d", &tpgid);
    
    // Skip fields 9 to 22
    for(int i = 0; i < 14; i++) {
        fscanf(fp, " %lu", (unsigned long*)&dummy_int); // Use lu to be safe with sizes
    }
    
    // Read VSize (23)
    fscanf(fp, " %lu", &vsize);
    
    fclose(fp);

    // 4. Determine Foreground/Background
    // A process is foreground if its Process Group == Terminal Process Group
    char status_str[4];
    if (pgrp == tpgid) {
        snprintf(status_str, 3, "%c+", state);
    } else {
        snprintf(status_str, 2, "%c", state);
    }

    // 5. Get Executable Path
    char exe_path[PATH_MAX];
    snprintf(stat_path, sizeof(stat_path), "/proc/%d/exe", pid); // reuse stat_path buffer
    
    ssize_t len = readlink(stat_path, exe_path, sizeof(exe_path) - 1);
    if (len != -1) {
        exe_path[len] = '\0';
        
        // Handle "~" replacement
        char final_path[PATH_MAX];
        if (strncmp(exe_path, home_dir, strlen(home_dir)) == 0) {
            snprintf(final_path, sizeof(final_path), "~%s", exe_path + strlen(home_dir));
        } else {
            strcpy(final_path, exe_path);
        }
        
        // Output
        printf("pid : %d\n", pid);
        printf("process status : %s\n", status_str);
        printf("Process Group : %d\n", pgrp);
        printf("Virtual memory : %lu\n", vsize);
        printf("executable path : %s\n", final_path);

    } else {
        // If process is a zombie or permission denied, exe might not be readable
        printf("pid : %d\n", pid);
        printf("process status : %s\n", status_str);
        printf("Process Group : %d\n", pgrp);
        printf("Virtual memory : %lu\n", vsize);
        printf("executable path : unknown\n");
    }
}