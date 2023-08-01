
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

char socket_path[] = "/tmp/lab6";

int main(int argc, char *argv[])
{
    struct sockaddr_un addr;
    char buf[100];
    int fd, rc;
    bool waiting = true;

    memset(&addr, 0, sizeof(addr));
    // Create the socket
    cout << "server: create socket" << endl;
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        cout << "client: " << strerror(errno) << endl;
        exit(-1);
    }

    addr.sun_family = AF_UNIX;
    // Set the socket path to a local socket file
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    // Connect to the local socket
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        cout << "client1: " << strerror(errno) << endl;
        close(fd);
        exit(-1);
    }
    cout << "client: connection success" << endl;


    // Receives from server
    while (waiting)
    {
        rc = read(fd, buf, sizeof(buf));

        if (strncmp(buf, "Pid", 5) == 0)
        {
            cout << "Client receives \"Pid\"" << endl;
            int pid = getpid();
            snprintf(buf, sizeof(buf), "%d", pid);
            cout << "Client sends back: " << '"' << buf << '"' << endl;

            if (write(fd, buf, sizeof(buf)) != sizeof(buf))
            {
                if (sizeof(buf) > 0)
                {
                    fprintf(stderr, "partial write");
                }
                else
                {
                    cout << "client: " << strerror(errno) << endl;
                    close(fd);
                    exit(-1);
                }
            }
        }

        if (strncmp(buf, "Sleep", 6) == 0)
        {
            cout << "Client receives: " << '"' << buf << '"' << endl;
            sleep(5);
            cout << "Client sends back: \"Done\"" << endl;
            if (write(fd, "Done", sizeof("Done")) != sizeof("Done"))
            {
                if (sizeof("Done") > 0)
                {
                    fprintf(stderr, "partial write");
                }
                else
                {
                    cout << "Client's error: " << strerror(errno) << endl;
                    close(fd);
                    exit(-1);
                }
            }
        }

        if (strncmp(buf, "Quit", 5) == 0)
        {
            cout << "Client receives: " << '"' << buf << '"' << endl;
            waiting = false;
        }
    }
    cout << "Client: Terminate" << endl;
    close(fd);
    return 0;
}