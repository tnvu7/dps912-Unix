#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

char socket_path[] = "/tmp/lab6";

int main(int argc, char *argv[])
{
    struct sockaddr_un addr;
    char buf[100];
    int fd, cl, rc;
    bool isRunning = true;

    memset(&addr, 0, sizeof(addr));
    // Create the socket
    cout << "server: create socket" << endl;
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        cout << "server: " << strerror(errno) << endl;
        exit(-1);
    }

    addr.sun_family = AF_UNIX;
    // Set the socket path to a local socket file
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
    unlink(socket_path);

    cout << "server: bind socket" << endl;
    // Bind the socket to this local socket file
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        cout << "server: " << strerror(errno) << endl;
        close(fd);
        exit(-1);
    }

    cout << "server: listening" << endl;
    // Listen for a client to connect to this local socket file
    if (listen(fd, 5) == -1)
    {
        cout << "server: " << strerror(errno) << endl;
        unlink(socket_path);
        close(fd);
        exit(-1);
    }

    // Accept the client's connection 
    if ((cl = accept(fd, NULL, NULL)) == -1)
    {
        cout << "server: " << strerror(errno) << endl;
        unlink(socket_path);
        close(fd);
        exit(-1);
    }
    cout << "server: connection success" << endl;

    //Sends commands to client
    cout << "server sends: \"Pid\"" << endl;
    write(cl, "Pid", 4);
    if ((rc = read(cl, buf, sizeof(buf))) > 0)
    {
        cout << "Server receives: " << '"' << buf << '"' << endl;
    }

    cout << "server sends: \"Sleep\"" << endl;
    write(cl, "Sleep", sizeof("Sleep"));

    if ((rc = read(cl, buf, sizeof(buf))) > 0)
    {
        cout << "Server receives: " << '"' << buf << '"' << endl;
    }

    cout << "server sends: \"Quit\"" << endl;
    write(cl, "Quit", sizeof("Quit"));

    cout << "server: terminate" << endl;
    unlink(socket_path);
    close(fd);
    close(cl);

    return 0;
}