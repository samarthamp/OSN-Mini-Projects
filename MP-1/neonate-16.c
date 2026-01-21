#include "header.h"
#include "neonate-16.h"
#include <termios.h>

// Helper to get the most recent PID from /proc/loadavg
int get_most_recent_pid() {
    FILE *fp = fopen("/proc/loadavg", "r");
    if (!fp) return -1;

    char buffer[256];
    if (fgets(buffer, sizeof(buffer), fp)) {
        // Format example: 0.03 0.05 0.06 1/456 12345
        // The last number (12345) is the most recent PID
        
        char *token = strtok(buffer, " ");
        char *last = token;
        while (token != NULL) {
            last = token;
            token = strtok(NULL, " ");
        }
        
        // 'last' now points to the PID string (e.g., "12345\n")
        fclose(fp);
        return atoi(last);
    }
    
    fclose(fp);
    return -1;
}

// Helper to disable raw mode (restore original settings)
void disable_raw_mode(struct termios *orig_termios) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, orig_termios);
}

// Helper to enable raw mode
void enable_raw_mode(struct termios *orig_termios) {
    tcgetattr(STDIN_FILENO, orig_termios);
    struct termios raw = *orig_termios;
    
    // Disable ICANON (canonical mode - line buffering) and ECHO (printing input)
    raw.c_lflag &= ~(ICANON | ECHO);
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void execute_neonate(char **args) {
    // 1. Parse Arguments
    if (args[1] == NULL || strcmp(args[1], "-n") != 0 || args[2] == NULL) {
        printf("Usage: neonate -n [time_arg]\n");
        return;
    }

    int time_arg = atoi(args[2]);
    if (time_arg < 0) {
        printf("Invalid time argument\n");
        return;
    }

    // 2. Setup Raw Mode
    struct termios orig_termios;
    enable_raw_mode(&orig_termios);

    // 3. Loop
    // Use select to handle both timing and input check
    while (1) {
        // Print the PID
        int pid = get_most_recent_pid();
        if (pid != -1) printf("%d\n", pid);
        else printf("Error reading PID\n");

        // Flush stdout to ensure it prints immediately
        fflush(stdout);

        // Setup select
        fd_set set;
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);

        struct timeval timeout;
        timeout.tv_sec = time_arg;
        timeout.tv_usec = 0;

        // Wait
        int res = select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout);

        if (res > 0) {
            // Input available
            char c;
            if (read(STDIN_FILENO, &c, 1) == 1) {
                if (c == 'x') {
                    disable_raw_mode(&orig_termios);
                    return; // Exit
                }
            }
        } else if (res == 0) {
            // Timeout -> Loop continues and prints PID again
        } else {
            // Error
            perror("neonate select");
            break;
        }
    }

    // Ensure raw mode is disabled if loop breaks abnormally
    disable_raw_mode(&orig_termios);
}