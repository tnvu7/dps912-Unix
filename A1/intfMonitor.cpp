#include <iostream>
#include <fstream>
#include <signal.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <filesystem>
#define GET_VARIABLE_NAME(Variable) (#Variable)
using namespace std;

char socket_path[]="/tmp/mySockA1";
bool is_running;
const int BUF_LEN=100;

string get_statistics(string file_path, string var_name, bool stats_path = true) {
    string value, path;
    if (stats_path) {
        path = file_path + "statistics/" + var_name;
    } else {
        path = file_path + var_name;
    }
    
    ifstream ifs(path);
    ifs >> value;
    ifs.close();
    return value;
}

void setLinkUp() {
    //Use IOCTL command
    struct ifreq ifr;
        strcpy(ifr.ifr_name, this->name.c_str());
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd >= 0)
        {
            if (ioctl(fd, SIOCGIFFLAGS, &ifr) == 0)
            {
                ifr.ifr_flags |= IFF_UP;
                if (ioctl(fd, SIOCSIFFLAGS, &ifr) == 0)
                {
                    cout << "Set Link Up successfully." << endl;
                }
                else
                {
                    cout << "Error set link up: " << strerror(errno) << endl;
                }
            }
            else
            {
                cout << "Error get link flags: " << strerror(errno) << endl;
            }
            close(fd);
        }
        else
        {
            cout << "Error: " << strerror(errno) << endl;
        }
}

int main(int argc, char *argv[]) {
    //Set up socket communications
    struct sockaddr_un addr;
    char buf[BUF_LEN];
    int len, ret;
    int fd,rc;
    string file_path = '';

    //File path to get statistics
    file_path = "/sys/class/net/" + argv[0] + "/";
    if (!filesystem::is_directory(file_path)) {
        cout << file_path << "path not found" << endl;
        exit(-1);
    }

    cout<<"DEBUG - client("<<getpid()<<"): running..."<<endl;
    memset(&addr, 0, sizeof(addr));

    //Create the socket
    if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        cout << "client(" << getpid() << "): "<<strerror(errno) << endl;
        exit(-1);
    }

    addr.sun_family = AF_UNIX;
    //Set the socket path to a local socket file
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    //Connect to the local socket
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        cout << "client("<<getpid()<<"): " << strerror(errno) << endl;
        close(fd);
        exit(-1);
    }

    write(fd, "Ready", sizeof("Ready"));

    int bytes = read(fd, buf, sizeof(buf));
    
    if (bytes > 0 && strcmp(buffer, "Monitor") == 0) {
        write(fd, "Monitoring", sizeof("Monitoring"));
        is_running=true;
    }

    //Stats variables
    string name;
    string operstate;
    int carrier_up_count;
    int carrier_down_count;
    int rx_bytes;
    int rx_dropped;
    int rx_errors;
    int rx_packets;
    int tx_bytes;
    int tx_dropped;
    int tx_errors;
    int tx_packets;

    while (is_running) {
        //Read file to get statistics
        string var_name = GET_VARIABLE_NAME(operstate);
        operstate = get_statistics(file_path, var_name, false);

        var_name = GET_VARIABLE_NAME(carrier_up_count);
        carrier_up_count = std::stoi(read_file(file_path, var_name, false));

        var_name = GET_VARIABLE_NAME(carrier_down_count);
        carrier_down_count = std::stoi(read_file(file_path, var_name, false));

        var_name = GET_VARIABLE_NAME(rx_bytes);
        rx_bytes = std::stoi(read_file(file_path, var_name));

        var_name = GET_VARIABLE_NAME(rx_dropped);
        rx_dropped = std::stoi(read_file(file_path, var_name));

        var_name = GET_VARIABLE_NAME(rx_errors);
        rx_errors = std::stoi(read_file(file_path, var_name));

        var_name = GET_VARIABLE_NAME(rx_packets);
        rx_packets = std::stoi(read_file(file_path, var_name));

        var_name = GET_VARIABLE_NAME(tx_bytes);
        tx_bytes = std::stoi(read_file(file_path, var_name));

        var_name = GET_VARIABLE_NAME(tx_dropped);
        tx_dropped = std::stoi(read_file(file_path, var_name));

        var_name = GET_VARIABLE_NAME(tx_errors);
        tx_errors = std::stoi(read_file(file_path, var_name));

        var_name = GET_VARIABLE_NAME(tx_packets);
        tx_packets = std::stoi(read_file(file_path, var_name));

        //TODO: Remove std::, check if + works for cout or use <<

        cout << "Interface: " + name + " state: " + operstate + " up_count: " + std::to_string(carrier_up_count) + " down_count: " + to_string(carrier_down_count) << endl;
        cout << "rx_bytes: " + std::to_string(rx_bytes) + " rx_dropped: " + std::to_string(rx_dropped) + " rx_errors: " + std::to_string(rx_errors) + " rx_packets: " + std::to_string(rx_packets) << endl;
        cout << "tx_bytes: " + std::to_string(tx_bytes) + " tx_dropped: " + std::to_string(tx_dropped) + " tx_errors: " + std::to_string(tx_errors) + " tx_packets: " + std::to_string(tx_packets) << endl << endl;

        if (operstate == "down") {
            write(fd, "Link Down", sizeof("Link Down"));

            int bytes = read(fd, buf, sizeof(buf));
    
            if (bytes > 0 && strcmp(buffer, "Set Link Up") == 0) {
                cout << "DEBUG: Interface " + interface.name + ": " << buf << endl << endl;
                setLinkUp();
            }
        }
        sleep(1);
    }

    if (!is_running) {
        write(fd, "Done", sizeof("Done"));
    }
    close(fd);

    return 0;
}