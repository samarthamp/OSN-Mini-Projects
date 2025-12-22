#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

void task1() {
    printf("--- Task 1: Variable Isolation ---\n");
    int x = 25;
    pid_t pid = fork(); // pid_t is defined in sys/types.h and is used for process IDs
    // this provides better portability across different systems

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child Process
        printf("[Child] Initial value of x: %d\n", x);
        x = 50;
        printf("[Child] New value of x: %d\n", x);
        exit(0);
    } else {
        // Parent Process
        wait(NULL); // Wait for child to finish to keep output clean
        printf("[Parent] Value of x after child finished: %d\n", x);
        printf("Observation: The variable 'x' is independent in both processes.\n\n");
    }
}

void task2() {
    printf("--- Task 2: Exec and File IO ---\n");
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child Process
        // We need to write the Parent PID to a file.
        // Since we are using exec(), we construct a shell command.
        char command[100];
        sprintf(command, "echo %d > ppid_store.txt", getppid());
        
        // Execute the command using sh
        execlp("sh", "sh", "-c", command, NULL);
        
        // If exec fails, this line is reached
        perror("Exec failed");
        exit(1);
    } else {
        // Parent Process
        wait(NULL);
        printf("Task 2 completed. PPID stored in 'ppid_store.txt'.\n\n");
    }
}

void task3() {
    printf("--- Task 3: Orphan Process ---\n");
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child Process
        // Sleep to ensure Parent dies first
        sleep(2); 
        
        printf("[Orphan Child] My Parent PID is now: %d\n", getppid());
        
        // Compare with file content
        FILE *f = fopen("ppid_store.txt", "r");
        if (f) {
            int file_ppid;
            fscanf(f, "%d", &file_ppid);
            printf("[Orphan Child] PID stored in file (Original Parent): %d\n", file_ppid);
            fclose(f);
            
            if (getppid() != file_ppid) {
                printf("Observation: The Parent PID has changed. I was adopted by init/systemd.\n");
            }
        }
        exit(0);
    } else {
        // Parent Process
        printf("[Parent] I am terminating now (PID: %d).\n", getpid());
        exit(0); // Parent dies immediately
    }
}

int main() {
    task1();
    task2();
    task3();
    return 0;
}