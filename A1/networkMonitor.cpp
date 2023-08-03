#include <iostream>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <vector>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

char socket_path[]="/tmp/mySockA1";
bool is_running;
const int BUF_LEN=100;
const int MAX_CLIENTS=2;

static void sigHandler(int sig)
{
    switch(sig) {
        case SIGINT:
            is_running=false;
            break;
    }
}

int main(int argc, char *argv[]) {
    //Create the socket for inter-process communications
    struct sockaddr_un addr;
    vector<int> childPids;
    int master_fd,max_fd = 0,clients[MAX_CLIENTS];
    fd_set readfds;
    int ret;
    char buf[BUF_LEN];

    cout << "DEBUG - pid: " << getpid() << endl;

    signal(SIGINT, sigHandler);

    //Fork child processors
    for (int i = 1; i < argc; i++) {
        int child = fork();
        
        if (child == -1) {
            cout << "Server - Error fork child: " << strerror(errno) << endl;
            exit(-1);
        } else if (child == 0) {
            childPids.push_back(getpid());
            execl("./intfMonitor", argv[i], (char *)NULL);
        }
    }

    //Create the socket
    cout << "DEBUG - server: create socket" << endl;
    memset(&addr, 0, sizeof(addr));
    if ( (master_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        cout << "server - Error create master socket: " << strerror(errno) << endl;
        exit(-1);
    }

    addr.sun_family = AF_UNIX;
    //Set the socket path to a local socket file
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
    unlink(socket_path);

    //Bind the socket to this local socket file
    if (bind(master_fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == -1) {
        cout << "server: " << strerror(errno) << endl;
        close(master_fd);
        exit(-1);
    }

    cout << "Waiting for the client..." << endl;
    //Listen for a client to connect to this local socket file
    if (listen(master_fd, argc - 1) == -1) {
        cout << "server: " << strerror(errno) << endl;
        unlink(socket_path);
        close(master_fd);
        exit(-1);
    }

    //Clear file descriptor set
    FD_ZERO(&readfds);

    //Add the master_fd to the socket set
    FD_SET(master_fd, &readfds);

    cout<<"client(s) connected to the server"<<endl;
    is_running = true;
    int numClients = 0;

    while(numClients < argc - 1) {
        //Create temporary file descriptor set
        fd_set activeSet = readfds;

        if (select(FD_SETSIZE, &activeSet, NULL, NULL, NULL) == -1) {
            cout << "server - Error: select " << strerror(errno) << endl;
            unlink(socket_path);
            close(master_fd);
            exit(-1);
        }

        //Incoming connection
        if(FD_ISSET(master_fd, &activeSet)) {
            //Accept client's connection
            int clientSoc = accept(master_fd, NULL, NULL);
            if (clientSoc == -1) {
                cout << "server - Error cannot accept client socket: " << strerror(errno) << endl;
                unlink(socket_path);
                close(master_fd);
                exit(-1);
            }
            FD_SET(clientSoc, &readfds);
            clients[numClients] = clientSoc;
            numClients++;
        }
    }

    //Unblock file descriptors from thread, allow read & write
    for (int i = 0; i < argc - 1; ++i)
    {
        fcntl(clients[i], F_SETFL, O_NONBLOCK);
    }

    //Keep running to wait for alert from clients, until user hit Ctrl C to terminate
    while(is_running) {
        for (int i = 0; i<MAX_CLIENTS; ++i) {
            int cl_soc = clients[i];

            if (FD_ISSET(cl_soc, &readfds)) {
                int bytes = read(cl_soc, buf, sizeof(buf));
                if (bytes < 0) {
                    continue;
                }
                if (strcmp(buf, "Ready") == 0) {
                    write(cl_soc, "Monitor", sizeof("Monitor"));
                }
                if (strcmp(buf, "Link Down") == 0) {
                    write(cl_soc, "Set Link Up", sizeof("Set Link Up"));
                }
                if (strcmp(buf, "Done") == 0)
                {
                    close(cl_soc);
                    FD_CLR(cl_soc, &readfds);
                }
            }
        }       
    }

    //Kill all child processors if user hit Ctrl C / terminate program
    if (!is_running) {
        for (int i = 0; i < childPids.size(); i++) {
            kill(childPids[i], SIGINT);
        }
    }

    unlink(socket_path);
    close(master_fd);
    //Close all clients' connections
    for (int i = 0; i < numClients; ++i)
    {
        cout << "Closing client's connection: " << clients[i] << endl;
        close(clients[i]);
    }
    
    return 0;
}