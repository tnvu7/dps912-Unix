#ifndef HEADER_H
#define HEADER_H

const char MEMNAME[]="MemTransfer";

// Shared memory structure
struct SharedData {
    int value;
    int count;
    int numbers[1000000];
};

#endif//HEADER_H