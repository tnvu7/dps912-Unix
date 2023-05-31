#include "pidUtil.h"
#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
    vector<int> pidList;
    int pid;
    string name;

    ErrStatus status = GetAllPids(pidList);
    if (status != Err_OK) {
        cout << "Error getting Pid. Error message: " << GetErrorMsg(status) << endl;
        return 1;
    }
    cout << "All Pids and Names\n-------------------------" << endl;

    for (int pid : pidList){
        status = GetNameByPid(pid, name);

        if (status != Err_OK) {
            cout << "Error getting name for PID " << pid << ". Error message: " << GetErrorMsg(status) << endl;
        } else {
            cout << "PID: " << pid << " - Name: " << name << endl;
        }
        
    }
    cout << "-------------------------\nPid 1 and name" << endl;

    pid = 1;
    status = GetNameByPid(pid,name);
    if (status == Err_NoPid) {
            cout << "Error getting name for PID " << pid << ". Error message: " << GetErrorMsg(status) << endl;
    } else {
        cout << "PID: " << pid << "- Name: " << name << endl;
    }

    cout << "-------------------------\nName Lab2 and Pid" << endl;

    name = "Lab2";
    status = GetPidByName(name, pid);
    if (status == Err_NoName) {
        cout << "Error message: " << GetErrorMsg(status) << endl;
    } else {
        cout << "PID: " << pid << " - Name: " << name << endl;
    }

    cout << "-------------------------\nName Lab22 and Pid" << endl;

    name = "Lab22";
    status = GetPidByName(name, pid);
    if (status == Err_NoName) {
        cout << "Error message: " << GetErrorMsg(status) << endl;
    } else {
        cout << "PID: " << pid << " - Name:" << name << endl;
    }

    return 0;
}

