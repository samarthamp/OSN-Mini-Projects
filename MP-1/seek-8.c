#include "header.h"
#include "seek-8.h"

// Helper to print file content for -e flag
void print_file_content(char *path) {
    if (access(path, R_OK) != 0) {
        printf("Missing permissions for task!\n");
        return;
    }

    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror("seek");
        return;
    }

    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), fp)) {
        printf("%s", buffer);
    }
    fclose(fp);
    printf("\n"); // Ensure newline at end
}

// Recursive function
void seek_recursive(char *target_dir_full, char *rel_prefix, char *search_target, 
                    int flag_d, int flag_f, int *count, char *last_match_full) {
    
    DIR *dir = opendir(target_dir_full);
    if (!dir) {
        // Permission denied or not a directory (can happen during recursion)
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and .. (hidden directories/files)
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construct Paths
        char new_full_path[PATH_MAX];
        char new_rel_path[PATH_MAX];
        
        snprintf(new_full_path, sizeof(new_full_path), "%s/%s", target_dir_full, entry->d_name);
        snprintf(new_rel_path, sizeof(new_rel_path), "%s/%s", rel_prefix, entry->d_name);

        struct stat st;
        if (stat(new_full_path, &st) == -1) continue;

        // Check Logic: Prefix Match OR Exact Match
        // Spec: "look for a file/folder with the exact name, or one which contains this target as a prefix"
        int name_match = (strncmp(entry->d_name, search_target, strlen(search_target)) == 0);

        if (name_match) {
            int is_dir = S_ISDIR(st.st_mode);
            int is_file = !is_dir;

            // Apply Filters
            if ((flag_d && is_dir) || (flag_f && is_file) || (!flag_d && !flag_f)) {
                
                // Store match for -e
                (*count)++;
                strcpy(last_match_full, new_full_path);

                // Print
                if (is_dir) {
                    printf("%s%s%s\n", COLOR_BLUE, new_rel_path, COLOR_RESET);
                } else {
                    printf("%s%s%s\n", COLOR_GREEN, new_rel_path, COLOR_RESET);
                }
            }
        }

        // Recurse if directory
        // Note: Spec says "Target directory's tree must be searched"
        if (S_ISDIR(st.st_mode)) {
            // Check access before recursing to avoid errors? 
            // opendir check at start of function handles it.
            seek_recursive(new_full_path, new_rel_path, search_target, flag_d, flag_f, count, last_match_full);
        }
    }
    closedir(dir);
}

void execute_seek(char **args, char *home_dir, char *prev_dir) {
    int flag_d = 0;
    int flag_f = 0;
    int flag_e = 0;
    char *search_target = NULL;
    char *target_dir_arg = NULL;

    // 1. Parse Arguments
    for (int i = 1; args[i] != NULL; i++) {
        if (args[i][0] == '-') {
            // Flags
            for (size_t j = 1; j < strlen(args[i]); j++) {
                if (args[i][j] == 'd') flag_d = 1;
                else if (args[i][j] == 'f') flag_f = 1;
                else if (args[i][j] == 'e') flag_e = 1;
            }
        } else {
            // Positional Args
            if (search_target == NULL) {
                search_target = args[i];
            } else if (target_dir_arg == NULL) {
                target_dir_arg = args[i];
            } else {
                // Ignore extra args or handle error?
            }
        }
    }

    // 2. Validation
    if (flag_d && flag_f) {
        printf("Invalid flags!\n");
        return;
    }
    if (search_target == NULL) {
        printf("seek: missing search target\n");
        return;
    }

    // 3. Resolve Target Directory
    char resolved_path[PATH_MAX];
    if (target_dir_arg == NULL) {
        strcpy(resolved_path, ".");
    } else {
        // Handle ~, -
        if (strcmp(target_dir_arg, "~") == 0) {
            strcpy(resolved_path, home_dir);
        } else if (strncmp(target_dir_arg, "~/", 2) == 0) {
            snprintf(resolved_path, sizeof(resolved_path), "%s/%s", home_dir, target_dir_arg + 2);
        } else if (strcmp(target_dir_arg, "-") == 0) {
             if (strlen(prev_dir) == 0) {
                printf("seek: OLDPWD not set\n");
                return;
            }
            strcpy(resolved_path, prev_dir);
        } else {
            strcpy(resolved_path, target_dir_arg);
        }
    }

    // 4. Start Search
    int match_count = 0;
    char last_match_full[PATH_MAX] = "";
    
    // The relative prefix for the start directory is "."
    seek_recursive(resolved_path, ".", search_target, flag_d, flag_f, &match_count, last_match_full);

    if (match_count == 0) {
        printf("No match found!\n");
        return;
    }

    // 5. Handle -e Flag
    if (flag_e && match_count == 1) {
        struct stat st;
        if (stat(last_match_full, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                // Check Execute Permission
                if (access(last_match_full, X_OK) != 0) {
                    printf("Missing permissions for task!\n");
                } else {
                    // Update prev_dir before changing (like hop)
                    char current_cwd[PATH_MAX];
                    if (getcwd(current_cwd, sizeof(current_cwd)) != NULL) {
                        strcpy(prev_dir, current_cwd);
                    }
                    if (chdir(last_match_full) != 0) {
                        perror("seek chdir");
                    } else {
                        // Normally hop prints path, seek -e behavior on directory is just "change directory"
                    }
                }
            } else {
                // File: Print Content
                print_file_content(last_match_full);
            }
        }
    }
}