#include "header.h"
#include "activities-13.h"

// Linked List Structure
typedef struct ProcessNode {
    pid_t pid;
    char command[1024];
    struct ProcessNode *next;
} ProcessNode;

static ProcessNode *head = NULL;

// Helper: Comparator for qsort
typedef struct {
    pid_t pid;
    char command[1024];
    char state[16];
} ProcessInfo;

int compare_processes(const void *a, const void *b) {
    ProcessInfo *p1 = (ProcessInfo *)a;
    ProcessInfo *p2 = (ProcessInfo *)b;
    return strcmp(p1->command, p2->command);
}

void add_process(pid_t pid, char *command_name) {
    ProcessNode *new_node = malloc(sizeof(ProcessNode));
    new_node->pid = pid;
    strcpy(new_node->command, command_name);
    new_node->next = head;
    head = new_node;
}

int remove_process(pid_t pid) {
    ProcessNode *current = head;
    ProcessNode *prev = NULL;

    while (current != NULL) {
        if (current->pid == pid) {
            if (prev == NULL) {
                head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            return 1;
        }
        prev = current;
        current = current->next;
    }
    return 0;
}

// Helper: Get state from /proc/[pid]/stat
// Returns 1 if running/stopped, 0 if process dead/zombie
int get_process_state(pid_t pid, char *state_str) {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    FILE *fp = fopen(path, "r");
    if (!fp) return 0; // Process likely finished

    char state_char;
    int dummy_int;
    char dummy_str[1024];

    // Format: pid (comm) state ...
    // Skip pid
    fscanf(fp, "%d", &dummy_int);
    // Skip comm (handle spaces)
    fscanf(fp, " %s ", dummy_str); 

    fscanf(fp, " %c", &state_char);
    
    fclose(fp);

    // Map State Char to Spec Requirement
    if (state_char == 'T') {
        strcpy(state_str, "Stopped");
    } else if (state_char == 'Z') {
        // Zombie processes should be reaped by signal handler, 
        // but if we see one here, it's effectively "Terminated"
        return 0; 
    } else {
        // R (Running), S (Sleeping), D (Disk Sleep) -> "Running"
        strcpy(state_str, "Running");
    }
    return 1;
}

void execute_activities() {
    // 1. Count processes
    int count = 0;
    ProcessNode *curr = head;
    while (curr) {
        count++;
        curr = curr->next;
    }
    printf("Total Background Jobs: %d\n", count);
    if (count == 0) return;

    // 2. Create Array for Sorting
    ProcessInfo *list = malloc(sizeof(ProcessInfo) * count);
    int valid_count = 0;

    curr = head;
    while (curr) {
        char state_str[16];
        if (get_process_state(curr->pid, state_str)) {
            list[valid_count].pid = curr->pid;
            strcpy(list[valid_count].command, curr->command);
            strcpy(list[valid_count].state, state_str);
            valid_count++;
        } else {
            // Process exists in our list but not in /proc (it died recently)
            // It will be cleaned up by sigchld_handler eventually
        }
        curr = curr->next;
    }

    // 3. Sort Lexicographically
    qsort(list, valid_count, sizeof(ProcessInfo), compare_processes);

    // 4. Print
    for (int i = 0; i < valid_count; i++) {
        printf("%d : %s - %s\n", list[i].pid, list[i].command, list[i].state);
    }

    free(list);
}

void kill_all_processes() {
    ProcessNode *curr = head;
    while (curr) {
        // Send SIGKILL to ensure they die
        kill(curr->pid, SIGKILL);
        curr = curr->next;
    }
}

int get_process_command(pid_t pid, char *dest) {
    ProcessNode *curr = head;
    while (curr) {
        if (curr->pid == pid) {
            strcpy(dest, curr->command);
            return 1;
        }
        curr = curr->next;
    }
    return 0;
}