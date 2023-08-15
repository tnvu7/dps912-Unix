#include <errno.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <net/if.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

const int PORT = 4201;
const int BUF_LEN = 4096;
bool is_running = true;
int fd;
char buf[BUF_LEN];
struct sockaddr_in server_addr, client_addr;
socklen_t socketlen = sizeof(client_addr);

pthread_mutex_t lock_x;
pthread_t receiveThread;
const char *log_path = "/tmp/A2LogFile";


void* receiveFunction(void *arg) {
    int thread_fd = *(int *)arg;

    // Set a socket timeout of 1 second
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    setsockopt(thread_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

    ofstream logFile(log_path, ofstream::out | ofstream::app);
    while (is_running)
    {
        memset(buf, 0, BUF_LEN);

        pthread_mutex_lock(&lock_x);
        
        if (!logFile.is_open()) {
            std::cerr << "Error opening log file." << std::endl;
            exit(-1);
        }
        //Receive data
        int msg_len = recvfrom(thread_fd, buf, BUF_LEN, 0, (struct sockaddr *)&client_addr, &socketlen);
        if (msg_len > 0) {
            // Write data from the buffer into the file
            logFile.write(buf, strlen(buf));
        } else {
            sleep(1);
        }
        
        // Unlock mutex
        pthread_mutex_unlock(&lock_x);
    }
    // Close the file when done
    logFile.close();
}


void sigHandler(int signal)
{
    if (signal == SIGINT)
    {
        is_running = false;
    }
}

void signalRegister()
{
    struct sigaction sigAction;
    sigAction.sa_handler = sigHandler;
    sigemptyset(&sigAction.sa_mask);
    sigAction.sa_flags = 0;
}

void setLogLevel(){
    string level;
    cout << "Choose log severity level:" << endl << "-------------------" << endl;
    cout << "1. DEBUG" << endl;
    cout << "2. WARNING" << endl;
    cout << "3. ERROR" << endl;
    cout << "4. CRITICAL" << endl;

    int user_input;
    cin >> user_input;

    switch (user_input) {
        case(1):
            level = "DEBUG";
            break;
        case(2):
            level = "WARNING";
            break;
        case(3):
            level = "ERROR";
            break;
        case(4):
            level = "CRITICAL";
            break;
        default:
            cout << "Option not available. Please choose again" << endl;
            break;
    }
    memset(buf,0,BUF_LEN);
    int len = sprintf(buf, "Set Log Level=%s", level.c_str()) + 1;
    
    //Send log level to client socket address
    int ret = sendto(fd, buf, len, 0, (struct sockaddr *)&client_addr, socketlen);
    
    if (ret < 0) {
        perror("ERROR: Send log to client");
    } else {
        cout << "Sent to client socket this log level: " << buf << endl;
    }
}

void dumpLogFile(){
    //open file for reading
    ifstream logFile(log_path);

    if (!logFile.is_open()) {
        cout << "ERROR: Open log file " << strerror(errno) << endl;
        exit(-1);
    }

    //Read and display content of the file
    string line;
    while(getline(logFile, line)) {
        cout << line << endl;
    }

    //Close the input file
    logFile.close();

}

int main(void) {
    // register shut down handler to listen for Ctrl C
    signalRegister();

    // Create the socket
    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0)
    {
            cout << "ERROR: Cannot create the socket: " << strerror(errno) << endl;
            return -1;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to this local socket file
    if (bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        cout << "ERROR: binding server socket: " << strerror(errno) << endl;
        exit(-1);
    }

    fcntl(fd, F_SETFL, O_NONBLOCK);
    // Initialize mutex lock
    pthread_mutex_init(&lock_x, NULL);

    // Create receive thread for each process
    int createResult = pthread_create(&receiveThread, NULL, receiveFunction, &fd);
    if (createResult != 0) {
        cout << "ERROR: Cannot create thread: " << strerror(errno) << endl;
        exit(-1);
    }

    // Run until user hit shutdown
    while(is_running) {
        cout << "Choose the following options:" << endl << "-------------------" << endl;
        cout << "1. Set log level" << endl;
        cout << "2. Dump log file" << endl;
        cout << "0. Shut down" << endl;

        int user_input;
        cin >> user_input;

        switch (user_input) {
            case(1):
                setLogLevel();
                break;
            case(2):
                dumpLogFile();
                break;
            case(0):
                is_running = false;
                break;
            default:
                cout << "Option not available. Please choose again" << endl;
        }
    }

    //Close thread, mutex, and file descriptor
    pthread_join(receiveThread, NULL);
    pthread_mutex_destroy(&lock_x);
    close(fd);

    return 0;
}