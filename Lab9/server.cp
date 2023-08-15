// server.cpp
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <cstring>
#include <arpa/inet.h>
#include <fcntl.h>
#include <queue>
#include <signal.h>

using namespace std;

const int BUF_LEN = 4096;
const int MAX_CLIENTS = 3;

bool is_running = true;
queue<string> messageQueue;
pthread_mutex_t messageQueueMutex = PTHREAD_MUTEX_INITIALIZER;

static void shutdownHandler(int sig) {
    switch (sig) {
        case SIGINT:
            is_running = false;
            break;
    }
}

void *recv_func(void *arg) {
    int fd = *(int *)arg;
    char buf[BUF_LEN];

    // read timeout set to 5 sec
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

    while (is_running) {
        int len = read(fd, buf, BUF_LEN - 1);

        // add msg to queue
        if (len > 0) {
            buf[len] = '\0';
            pthread_mutex_lock(&messageQueueMutex);
            messageQueue.push(string(buf));
            pthread_mutex_unlock(&messageQueueMutex);
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    // invalid args
    if (argc < 2) {
        cout << "usage: server <port number>" << endl;
        return -1;
    }

    cout << "server: running..." << endl;

    // ctrl-c handler
    struct sigaction action;
    action.sa_handler = shutdownHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);

    int master_fd, conn_fd[MAX_CLIENTS];
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    pthread_t tid[MAX_CLIENTS];

    if ((master_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cout << "server: " << strerror(errno) << endl;
        exit(-1);
    }

    // make master socket non-blocking
    int flags = fcntl(master_fd, F_GETFL, 0);
    fcntl(master_fd, F_SETFL, flags | O_NONBLOCK);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[1]));
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    // bind to socket
    if (bind(master_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        cout << "server: " << strerror(errno) << endl;
        close(master_fd);
        exit(-1);
    }

    // listen for client connections, max 3
    if (listen(master_fd, 3) < 0) {
        cout << "server: " << strerror(errno) << endl;
        close(master_fd);
        exit(-1);
    }

    int clients = 0;

    while (is_running) {
        // accept more clients if not all have been accepted
        if (clients < MAX_CLIENTS) {
            conn_fd[clients] = accept(master_fd, (struct sockaddr *)&addr, &addrlen);

            if (conn_fd[clients] > 0) {
                // client wants to connect, create a receive thread for the client
                if (pthread_create(&tid[clients], NULL, recv_func, &conn_fd[clients]) != 0) {
                    cout << "Cannot create receive thread" << endl;
                    cout << strerror(errno) << endl;
                    close(conn_fd[clients]);
                    return -1;
                }
                clients++;
            }
        }

        // print anything in the message queue
        while (!messageQueue.empty()) {
            pthread_mutex_lock(&messageQueueMutex);
            cout << messageQueue.front() << endl;
            messageQueue.pop();
            pthread_mutex_unlock(&messageQueueMutex);
        }

        sleep(1);
    }

    // send Quit to clients
    for (int i = 0; i < clients; ++i) {
        write(conn_fd[i], "Quit", strlen("Quit"));
        pthread_join(tid[i], NULL);
        close(conn_fd[i]);
    }

    cout << endl << "server: stopping..." << endl;
    close(master_fd);

    return 0;
}