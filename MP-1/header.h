#ifndef HEADERS_H
#define HEADERS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <pwd.h>
#include <sys/wait.h> // For waitpid
#include <signal.h>   // For signal handling
#include <sys/stat.h> // For stat()
#include <dirent.h>   // For opendir(), readdir()
#include <grp.h>      // For getgrgid()
#include <time.h>     // For strftime()
#include <fcntl.h> // Added for I/O Redirection
#include <termios.h>

// Networking Headers for iMan
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define COLOR_GREEN "\033[1;32m"
#define COLOR_BLUE  "\033[1;34m"
#define COLOR_WHITE "\033[1;37m"
#define COLOR_RESET "\033[0m"

#define PATH_MAX 4096

#define MAX_BG 1024

typedef struct {
    pid_t pid;
    char name[256];
} bg_job;

extern bg_job bg_jobs[MAX_BG];
extern int bg_count;

#endif