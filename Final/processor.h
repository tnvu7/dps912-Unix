#ifndef PROCESSOR_H
#define PROCESSOR_H

const char MEMNAME[]="MemTransfer";

// Shared memory key
#define SHM_KEY 1234

// Number of threads
#define NUM_THREADS 5

// Number of random numbers to generate
#define NUM_RANDOM_NUMBERS (NUM_THREADS * 2000)

#endif//PROCESSOR_H