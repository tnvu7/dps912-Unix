#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include "tnvu7.h"

// The device file for our driver. We will communicate
// with this driver through this file.
const char devFile[]="/dev/tnvu7";
const char logFile[]="logFile.txt";
const int  BUF_LEN=512;

using namespace std;

void sigHandler(int sig)
{
    pid_t pid = getpid();
    pid_t ppid = getppid();
    
    FILE *log_file = fopen(logFile, "a");
    int fdLog=open(logFile, O_RDWR);
    int fdDev=open(devFile, O_RDWR);
    if (log_file == NULL) {
        perror("Error: Cannot open log file");
        exit(1);
    }
    
    if (pid == 0) {
        fprintf(log_file, "Process ID (Child): %d\n", pid);
    } else {
        fprintf(log_file, "Process ID (Parent): %d\n", ppid);
    }
    
    if (sig == SIGINT) {        
        int channel_index;
        if (ioctl(fdLog, MIDTERM_GET_CHANNEL_INDEX, &channel_index) < 0) {
            perror("Failed to get channel index");
            exit(1);
        }
        
        fprintf(log_file, "Signal: SIGINT\nChannel Index: %d\n", channel_index);
    } else if (sig == SIGTSTP) {
        fprintf(log_file, "Signal: SIGTSTP\n");

    }
    fclose(log_file);
    close(fdLog);
    close(fdDev); //this will call midterm_tnvu7_close()
    
    exit(0);
}


int main()
{
    int fd, fd2 = 0;
    char buf[BUF_LEN];
    int  nbytes;
    int  perIndex;
    PERIPHERAL_INFO perInfo;

    fd=open(devFile, O_RDWR);
    if(fd < 0) {
        printf("Cannot open the device file...\n");
        return -1;
    }

    fd2=open(logFile, O_RDWR);
    if(fd2 < 0) {
        printf("Cannot open the log file...\n");
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        printf("Fork failed\n");
        return -1;
    }

    if(pid == 0) {
        //Child process
        printf("Child Process ID: %d\n", getpid());
        printf("Parent Process ID: %d\n", getppid());

        char *args[] = { "/bin/pwd", NULL };
        execvp(args[0], args);
        
        int new_channel_index = 6;
        if (ioctl(fd, MIDTERM_SET_CHANNEL_INDEX, &new_channel_index) < 0) {
            perror("Failed to set channel index");
            return -1;
        }
    } else {
        //Parent process
        printf("Child Process ID: %d\n", getpid());
        printf("Parent Process ID: %d\n", getppid());

        char *args[] = { "/bin/date", NULL };
        execvp(args[0], args);

        struct peripheral_info info;
        if (ioctl(fd, MIDTERM_GET_INFO, &info) < 0) {
            perror("Failed to get peripheral information");
            return -1;
        }
    }

    if (signal(SIGINT, sigHandler) == SIG_ERR || signal(SIGTSTP, sigHandler) == SIG_ERR) {
        exit(1);
    }

    close(fd);
    close(fd2);

    return 0;
}