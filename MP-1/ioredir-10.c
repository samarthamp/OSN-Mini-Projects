#include "header.h"
#include "ioredir-10.h"

int handle_redirection(char *cmd) {
    int append_mode = 0;

    // We scan the string manually to find <, >, >>
    // We will replace found symbols and filenames with spaces to "erase" them from the command string
    
    // 1. Scan for Input Redirection '<'
    char *in_pos = strchr(cmd, '<');
    if (in_pos) {
        *in_pos = ' '; // Replace symbol
        in_pos++;
        while (*in_pos == ' ' || *in_pos == '\t') in_pos++; // Skip whitespace
        
        if (*in_pos != '\0') {
            // Find end of filename
            char *end = in_pos;
            while (*end != ' ' && *end != '\t' && *end != '\0' && *end != '>' && *end != '<') end++;
            // Don't null terminate yet, just remember length or handle tokens carefully.
            // Simpler approach: Tokenize logic is hard in-place. 
            // We will extract filename and then memset the region to spaces.
            
            int len = end - in_pos;
            char *filename = malloc(len + 1);
            strncpy(filename, in_pos, len);
            filename[len] = '\0';
            
            // Clean up the string (memset to space)
            memset(in_pos, ' ', len);
            
            // Open Input File
            int fd = open(filename, O_RDONLY);
            if (fd < 0) {
                printf("No such input file found!\n");
                free(filename);
                return -1;
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
            free(filename);
        }
    }

    // 2. Scan for Output Redirection '>' or '>>'
    // We must check for '>>' before '>'
    char *out_pos = strstr(cmd, ">>");
    if (out_pos) {
        append_mode = 1;
        out_pos[0] = ' '; 
        out_pos[1] = ' ';
        out_pos += 2;
    } else {
        out_pos = strchr(cmd, '>');
        if (out_pos) {
            append_mode = 0;
            *out_pos = ' ';
            out_pos++;
        }
    }

    if (out_pos) {
        while (*out_pos == ' ' || *out_pos == '\t') out_pos++;
        if (*out_pos != '\0') {
            char *end = out_pos;
            while (*end != ' ' && *end != '\t' && *end != '\0' && *end != '>' && *end != '<') end++;
            
            int len = end - out_pos;
            char *filename = malloc(len + 1);
            strncpy(filename, out_pos, len);
            filename[len] = '\0';
            
            memset(out_pos, ' ', len);

            // Open Output File
            int flags = O_WRONLY | O_CREAT;
            if (append_mode) flags |= O_APPEND;
            else flags |= O_TRUNC;

            int fd = open(filename, flags, 0644);
            if (fd < 0) {
                perror("output redirection");
                free(filename);
                return -1;
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            free(filename);
        }
    }
    
    return 0;
}

void restore_io(int saved_stdin, int saved_stdout) {
    if (saved_stdin != -1) {
        dup2(saved_stdin, STDIN_FILENO);
        close(saved_stdin);
    }
    if (saved_stdout != -1) {
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
    }
}