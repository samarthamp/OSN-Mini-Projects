#include "header.h"
#include "reveal-4.h"

void print_details(struct stat fileStat) {
    printf(S_ISDIR(fileStat.st_mode) ? "d" : "-");
    printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
    printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
    printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");

    printf(" %lu", fileStat.st_nlink);

    struct passwd *pw = getpwuid(fileStat.st_uid);
    struct group  *gr = getgrgid(fileStat.st_gid);
    printf(" %s %s", pw ? pw->pw_name : "user", gr ? gr->gr_name : "group");

    printf(" %5ld", fileStat.st_size);

    char timebuf[80];
    struct tm *info = localtime(&fileStat.st_mtime);
    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", info);
    printf(" %s ", timebuf);
}

void execute_reveal(char **args, char *home_dir, char *prev_dir) {
    int show_a = 0;
    int show_l = 0;
    char *target_path_arg = NULL;

    for (int i = 1; args[i] != NULL; i++) {
        if (args[i][0] == '-') {
            if (strlen(args[i]) == 1 && args[i][0] == '-') {
                target_path_arg = args[i];
            } else {
                for (size_t j = 1; j < strlen(args[i]); j++) {
                    if (args[i][j] == 'a') show_a = 1;
                    else if (args[i][j] == 'l') show_l = 1;
                }
            }
        } else {
            target_path_arg = args[i];
        }
    }

    char path[PATH_MAX];
    if (target_path_arg == NULL) {
        strcpy(path, ".");
    } else if (strcmp(target_path_arg, "~") == 0) {
        strcpy(path, home_dir);
    } else if (strcmp(target_path_arg, "-") == 0) {
        if (strlen(prev_dir) == 0) {
            printf("reveal: OLDPWD not set\n");
            return;
        }
        strcpy(path, prev_dir);
    } else if (strncmp(target_path_arg, "~/", 2) == 0) {
        snprintf(path, sizeof(path), "%s/%s", home_dir, target_path_arg + 2);
    } else {
        strcpy(path, target_path_arg);
    }

    struct dirent **namelist;
    int n = scandir(path, &namelist, NULL, alphasort);
    
    if (n < 0) {
        struct stat s;
        if (stat(path, &s) == 0 && !S_ISDIR(s.st_mode)) {
            printf("%s\n", path);
            return;
        }
        perror("reveal: cannot access");
        return;
    }

    // PATH_MAX covers the directory, +256 covers the filename and slash.
    char full_path[PATH_MAX + 256]; 

    if (show_l) {
        long total_blocks = 0;
        for (int i = 0; i < n; i++) {
            if (!show_a && namelist[i]->d_name[0] == '.') continue;
            
            snprintf(full_path, sizeof(full_path), "%s/%s", path, namelist[i]->d_name);
            struct stat fileStat;
            if (stat(full_path, &fileStat) == 0) {
                total_blocks += fileStat.st_blocks;
            }
        }
        printf("total %ld\n", total_blocks / 2);
    }

    for (int i = 0; i < n; i++) {
        if (!show_a && namelist[i]->d_name[0] == '.') {
            free(namelist[i]);
            continue;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", path, namelist[i]->d_name);
        
        struct stat sb;
        if (stat(full_path, &sb) == -1) {
             printf("%s\n", namelist[i]->d_name);
             free(namelist[i]);
             continue;
        }

        if (show_l) {
            print_details(sb);
        }

        if (S_ISDIR(sb.st_mode)) {
            printf("%s%s%s\n", COLOR_BLUE, namelist[i]->d_name, COLOR_RESET);
        } else if (sb.st_mode & S_IXUSR) {
            printf("%s%s%s\n", COLOR_GREEN, namelist[i]->d_name, COLOR_RESET);
        } else {
            printf("%s%s%s\n", COLOR_WHITE, namelist[i]->d_name, COLOR_RESET);
        }

        free(namelist[i]);
    }
    free(namelist);
}