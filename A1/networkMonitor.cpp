#include <iostream>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

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

    for (int i = 1; i < argc; i++) {
        int child = fork();
        
        if (child < 0) {
            cout << "server: " << strerror(errno) << endl;
            exit(-1);
        } else {
            cout << "DEBUG - server ppid: " << getpid() << endl;
            childPids.push_back(getpid());
            cout << "DEBUG - server: execute " << argv[i] << endl;
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

    while(numClients < 2) {
        //Create temporary file descriptor set
        fd_set activeSet = readfds;

        ret = select(max_fd+1, &readfds, NULL, NULL, NULL);

        //Incoming connection
        if(FD_ISSET(master_fd, &readfds)) {
            int clientSoc = accept(master_fd, NULL, NULL);
            if (clientSoc == -1) {
                cout << "server: " << strerror(errno) << endl;
                unlink(socket_path);
                close(master_fd);
                exit(-1);
            }
            cout << "server: incoming connection " << clientSoc << endl;
            clients[numClients] = clientSoc;
            numClients++;

            // Break the loop if we have reached the desired number of clients
            // if (numClients == 2)
            //     break;
        }
    }

    //Unblock file descriptors from thread, allow read & write
    for (int i = 0; i < argc - 1; ++i)
    {
        fcntl(clients[i], F_SETFL, O_NONBLOCK);
    }

    while(is_running) {
        for (int i = 0; i<MAX_CLIENTS; ++i) {
            int cl_soc = clients[i];

            ret =  FD_ISSET(cl_soc, &readfds);
            if (ret!=0) {
                int bytes = read(cl_soc, buf, sizeof(buf));
                if (bytes < 0) {
                    continue;
                }

                if (strcmp(buf, "Ready") == 0) {
                    write(master_fd, "Monitor", sizeof("Monitor"));
                }
                if (strcmp(buf, "Link Down") == 0) {
                    write(master_fd, "Set Link Up", sizeof("Set Link Up"));
                }
                if (strcmp(buf, "Done") == 0)
                {
                    close(cl_soc);
                    FD_CLR(cl_soc, &readfds);
                }
            }
        }       
    }

    if (!is_running) {
        for (int i = 0; i < childPids.size(); i++) {
            kill(childPids[i], SIGINT);
        }
    }

    unlink(socket_path);
    close(master_fd);
    for (int i = 0; i < numClients; ++i)
    {
        close(clients[i]);
    }
    
    return 0;
}