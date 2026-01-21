#include "header.h"
#include "prompt-1.h"

void display_prompt(char *home_dir) {
    // 1. Get Username
    // We use getpwuid with getuid() to be safe, though getenv("USER") is simpler but spoofable.
    struct passwd *pw = getpwuid(getuid());
    char *username = pw ? pw->pw_name : "unknown";

    // 2. Get System Name (Hostname)
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);

    // 3. Get Current Working Directory
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        return;
    }

    // 4. Handle Directory Formatting
    // We need to check if the CWD starts with the home_dir path
    char display_path[PATH_MAX];
    
    int home_len = strlen(home_dir);
    int cwd_len = strlen(cwd);

    // Logic: 
    // If cwd == home_dir -> "~"
    // If cwd starts with home_dir AND the next char is '/' -> "~/..."
    // Else -> absolute path
    
    if (strncmp(cwd, home_dir, home_len) == 0) {
        if (cwd_len == home_len) {
            // Exactly the home directory
            strcpy(display_path, "~");
        } 
        else if (cwd[home_len] == '/') {
            // Subdirectory of home
            sprintf(display_path, "~%s", cwd + home_len);
        } 
        else {
            // Case like /home/user2 when home is /home/user 
            // (Shared prefix but different folder)
            strcpy(display_path, cwd);
        }
    } else {
        // Outside of home directory
        strcpy(display_path, cwd);
    }

    // 5. Print Final Prompt
    // Format: <Username@SystemName:CurrentDir>
    printf("<%s@%s:%s>", username, hostname, display_path);
}