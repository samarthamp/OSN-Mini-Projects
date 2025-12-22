#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define FILENAME "newfile.txt"

void handle_input() {
    char buffer[1024];
    // Consume the space or " -> " after INPUT if strictly following the visual format,
    // but here we just grab the rest of the line.
    scanf(" %[^\n]", buffer); 

    int fd = open(FILENAME, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd < 0) {
        perror("Error opening file");
        return;
    }

    // Append newline to the input
    strcat(buffer, "\n");
    
    if (write(fd, buffer, strlen(buffer)) < 0) {
        perror("Error writing to file");
    }
    
    close(fd);
}

void handle_print() {
    char buffer[1024];
    int fd = open(FILENAME, O_RDONLY);
    if (fd < 0) {
        // If file doesn't exist yet, just return or print nothing
        perror("Error opening file for reading");
        return;
    }

    int bytes_read;
    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }

    if (bytes_read < 0) {
        perror("Error reading file");
    }

    close(fd);
}

int main() {
    // Ensure file exists on startup
    int fd = open(FILENAME, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Startup error");
        exit(1);
    }
    close(fd);

    char command[100];

    while (1) {
        // Simple prompt logic
        scanf("%s", command);

        if (strcmp(command, "INPUT") == 0) {
            handle_input();
        } else if (strcmp(command, "PRINT") == 0) {
            handle_print();
        } else if (strcmp(command, "STOP") == 0) {
            break;
        } else {
            printf("Invalid Command. Use INPUT, PRINT, or STOP.\n");
        }
    }

    return 0;
}