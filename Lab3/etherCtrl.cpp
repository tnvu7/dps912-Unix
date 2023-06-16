#include <errno.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <unistd.h>

#define NAME_SIZE 16

using namespace std;

int main()
{
    int fd;
    int ret;
    int selection;
    struct ifreq ifr;
    char if_name[NAME_SIZE];
    unsigned char *mac=NULL;
    struct sockaddr_in *addr=NULL;

    cout << "Enter the interface name: ";
    cin >> if_name;

    size_t if_name_len=strlen(if_name);
    if (if_name_len<sizeof(ifr.ifr_name)) {
        memcpy(ifr.ifr_name, if_name, if_name_len);
        ifr.ifr_name[if_name_len]=0;//NULL terminate
    } else {
        cout << "Interface name is too long!" << endl;
	return -1;
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd<0) {
        cout << strerror(errno);
	return -1;
    }

    system("clear");
    do {
        cout << "Choose from the following:" << endl;
	cout << "1. Hardware address" << endl;
	cout << "2. IP address" << endl;
	cout << "3. Network mask" << endl;
	cout << "4. Broadcast address" << endl;
	cout << "0. Exit" << endl << endl;
	cin >> selection;
	switch(selection) {
        case 1:
            if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1) {
                cout << strerror(errno) << endl;
                close(fd);
                break;
            }
            mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
            cout << "Hardware address: ";
            for (int i = 0; i < 6; ++i) {
                cout << static_cast<int>(mac[i]);
                if (i < 5) {
                    cout << ":";
                }
            }
            cout << endl;
            break;
        case 2:
            if (ioctl(fd, SIOCGIFADDR, &ifr) == -1) {
                cout << strerror(errno) << endl;
                close(fd);
                break;
            }
            addr = (sockaddr_in*)&ifr.ifr_addr;
            char ip_address[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(addr->sin_addr), ip_address, INET_ADDRSTRLEN);
            std::cout << "IP address: " << ip_address << std::endl;
            break;
        case 3:
        if (ioctl(fd, SIOCGIFNETMASK, &ifr) == -1) {
                cout << strerror(errno) << endl;
                close(fd);
                break;
            }
            addr = (sockaddr_in *)&ifr.ifr_netmask;
            char network_mask[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(addr->sin_addr), network_mask, INET_ADDRSTRLEN);
            cout << "Network mask: " << network_mask << endl;
            break;
        case 4:
            if (ioctl(fd, SIOCGIFBRDADDR, &ifr) == -1) {
                cout << strerror(errno) << endl;
                close(fd);
                break;
            }
            addr = (sockaddr_in *)&ifr.ifr_broadaddr;
            char broadcast_address[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(addr->sin_addr), broadcast_address, INET_ADDRSTRLEN);
            cout << "Broadcast address: " << broadcast_address << endl;
            break;
        }
	if(selection!=0) {
            char key;
            cout << "Press any key to continue: ";
            cin >> key;
            system("clear");
        }
    } while (selection!=0);


    close(fd);
    return 0;
}
