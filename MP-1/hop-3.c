#include "header.h"
#include "hop-3.h"

void execute_hop(char **args, char *home_dir, char *prev_dir) {
    char current_cwd[PATH_MAX];
    if (getcwd(current_cwd, sizeof(current_cwd)) == NULL) {
        perror("hop: getcwd failed");
        return;
    }

    // No args -> home
    if (args[1] == NULL) {
        if (chdir(home_dir) != 0) {
            perror("hop");
        } else {
            strcpy(prev_dir, current_cwd); 
            printf("%s\n", home_dir);
        }
        return;
    }

    int i = 1;
    while (args[i] != NULL) {
        char target_path[PATH_MAX];
        char *arg = args[i];

        if (strcmp(arg, "~") == 0) {
            strcpy(target_path, home_dir);
        } else if (strcmp(arg, "-") == 0) {
            if (strlen(prev_dir) == 0) {
                printf("hop: OLDPWD not set\n");
                i++;
                continue;
            }
            strcpy(target_path, prev_dir);
        } else if (strncmp(arg, "~/", 2) == 0) {
            snprintf(target_path, sizeof(target_path), "%s/%s", home_dir, arg + 2);
        } else {
            strcpy(target_path, arg);
        }

        char temp_current[PATH_MAX];
        getcwd(temp_current, sizeof(temp_current));

        if (chdir(target_path) != 0) {
            perror("hop");
        } else {
            // Update shared prev_dir
            strcpy(prev_dir, temp_current);
            
            char new_cwd[PATH_MAX];
            if (getcwd(new_cwd, sizeof(new_cwd)) != NULL) {
                printf("%s\n", new_cwd);
            }
        }
        i++;
    }
}