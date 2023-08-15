#include <arpa/inet.h>
#include <iostream>
#include <net/if.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include "Logger.h"

using namespace std;

const int PORT = 4201;
const char IP_ADDR[] = "127.0.0.1"; // Address of the server
const int BUF_LEN = 4096;
bool is_running = true;
char buf[BUF_LEN];
int fd, ret, len;
struct sockaddr_in addr;
socklen_t addrlen;
LOG_LEVEL g_level;
pthread_t receiveThread;
pthread_mutex_t lock_x;


void* receiveFunction(void *arg) {
    int fd = *(int *)arg;

    // Set a socket timeout of 1 second
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

    while (is_running)
    {
        memset(buf, 0, BUF_LEN);
        pthread_mutex_lock(&lock_x);
        int msg_len = recvfrom(fd, buf, BUF_LEN, 0, (struct sockaddr *)&addr, &addrlen);
        string msg = buf;
        if (msg_len > 0) {
            string log = msg.substr((msg.find("=")+1));
            if (log == "DEBUG") {
                g_level = DEBUG;
            } 
            else if (log == "WARNING") {
                g_level = WARNING;
            } 
            else if (log == "ERROR") {
                g_level = ERROR;
            } 
            else if (log == "CRITICAL") {
                g_level = CRITICAL;
            } 
            else continue;
        } else {
            sleep(1);
        }
        msg = "";
        // Unlock shared resources
        pthread_mutex_unlock(&lock_x);
    }
    //pthread_exit(NULL);
}

int InitializeLog() {
    // Create the socket
    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0)
    {
            cout << "ERROR: Cannot create the socket: " << strerror(errno) << endl;
            exit(EXIT_FAILURE);
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(IP_ADDR);
    addr.sin_port = htons(PORT);

    addrlen = sizeof(addr);

    // Initialize mutex lock
    pthread_mutex_init(&lock_x, NULL);

    is_running = true;
    // Create receive thread for each process
    int createResult = pthread_create(&receiveThread, NULL, receiveFunction, &fd);
    if (createResult < 0) {
        cout << "ERROR: Cannot create thread: " << strerror(errno) << endl;
        return -1;
    }

    return 0;
}
void SetLogLevel(LOG_LEVEL level) {
    g_level = level;
}
void Log(LOG_LEVEL level, const char *prog, const char *func, int line, const char *message){
    if (level >= g_level) {
        time_t now = time(0);
        char *dt = ctime(&now);
        memset(buf,0,BUF_LEN);
        char levelStr[][16]={"DEBUG", "WARNING", "ERROR", "CRITICAL"};
        int d_len = sprintf(buf, "%s %s %s:%s:%d %s\n", dt, levelStr[level], prog, func, line, message) + 1;
        buf[d_len-1]='\0';

        sendto(fd, buf, d_len, 0, (struct sockaddr *)&addr, sizeof(addr));
    }
}
void ExitLog(){
    is_running = false;
    pthread_join(receiveThread, NULL);
    pthread_mutex_destroy(&lock_x);
    close(fd);
}