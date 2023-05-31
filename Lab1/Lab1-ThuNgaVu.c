#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_SIZE 256

int isDigit(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return 0;
        }
    }
    return 1;
}

void readFileAndPrint(const char *pid) {
    char statusPath[MAX_SIZE];
    sprintf(statusPath, "/proc/%s/status", pid);
    FILE *statusFile = fopen(statusPath, "r");

    if (statusFile != NULL) {
        char line[MAX_SIZE];
        char processName[MAX_SIZE];
        char processID[MAX_SIZE];
        int memoryUsage = -1;

        while (fgets(line, MAX_SIZE, statusFile) != NULL) {
            if (strncmp(line, "Name:", 5) == 0) {
                sscanf(line + 5, "%s", processName);
            } else if (strncmp(line, "Pid:", 4) == 0) {
                sscanf(line + 4, "%s", processID);
            } else if (strncmp(line, "VmRSS:", 6) == 0) {
                sscanf(line + 6, "%d", &memoryUsage);
            }
        }

        fclose(statusFile);

        if (memoryUsage > 10000) {
            printf("Process Name: %s\n", processName);
            printf("Process ID: %s\n", processID);
            printf("Memory Usage: %d kB\n", memoryUsage);
            printf("*************************\n");
        }
    } else {
        perror("Failed to open file status");
    }
}

int main() {
    DIR *procDir = opendir("/proc");

    if (procDir == NULL) {
        perror("Failed to open /proc directory.");
        return 1;
    }

    struct dirent *input;
    while ((input = readdir(procDir)) != NULL) {
        if (isDigit(input->d_name)) {
            readFileAndPrint(input->d_name);
        }
    }

    closedir(procDir);
    return 0;
}
