#include <iostream>
#include <cstdlib>
#include <ctime>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <fstream>
#include "processor.h"

using namespace std;

// Shared data structure
struct SharedData {
    int count;
    int numbers[NUM_RANDOM_NUMBERS];
};

// Global variables
SharedData* sharedData;
sem_t* semaphore;

// Mutex for pipeline synchronization
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Thread function to generate random numbers
void* threadFunction(void* arg) {
    int threadId = *(int*)arg;
    int startIndex = threadId * 2000;

    for (int i = 0; i < 2000; ++i) {
        int randomNumber = rand() % 1000;
        cout << "Thread#" << threadId << ": Thread<" << threadId << "> Number generated " << randomNumber << " to pipeline processing." << endl;

        pthread_mutex_lock(&mutex);
        sharedData->numbers[startIndex + i] = randomNumber;
        sharedData->count++;
        pthread_mutex_unlock(&mutex);

        usleep(1000);
    }

    return nullptr;
}

// Child process function to read from the pipeline and store in shared memory
void childProcess() {
    int totalCount = 0;

    while (true) {
        // Wait for the semaphore to be posted by the threads
        sem_wait(semaphore);

        // Read data from the pipeline and store in shared memory
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < sharedData->count; ++i) {
            sharedData->numbers[totalCount++] = sharedData->numbers[i];
        }
        sharedData->count = 0;
        pthread_mutex_unlock(&mutex);

        if (totalCount >= 1000000) {
            cout << "Child Process: Total random numbers >= 1,000,000. Exiting..." << endl;
            break;
        }
    }
}

// Generator process function
void generatorProcess() {
    // Create threads
    pthread_t threads[NUM_THREADS];
    int threadIds[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; ++i) {
        threadIds[i] = i;
        pthread_create(&threads[i], nullptr, threadFunction, &threadIds[i]);
    }

    // Fork child process
    pid_t childPid = fork();
    if (childPid == 0) {
        // Child process
        childProcess();
        exit(0);
    }

    // Wait for threads to finish
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], nullptr);
    }

    // Notify child process to exit
    sem_post(semaphore);

    // Wait for child process to finish
    waitpid(childPid, nullptr, 0);
}

// Monitor process function
void monitorProcess() {
    while (true) {
        sem_wait(semaphore);
        if (sharedData->count > 0) {
            // Write data to log file
            string logFileName = "132165192_Natalie.log";
            ofstream logFile(logFileName);

            for (int i = 0; i < sharedData->count; ++i) {
                logFile << sharedData->numbers[i] << endl;
            }

            logFile.close();

            // Reset count in shared memory
            sharedData->count = 0;
        } else {
            cout << "Parent Process: No data in shared memory. Signaling child process..." << endl;
            sem_post(semaphore);
        }

        // Sleep for a while before checking again
        sleep(5);
    }
}

int main() {
    const char MEMNAME[]="MemTransfer";
    key_t          ShmKey;
    ShmKey=ftok(MEMNAME, 65);
    // Create or get a shared memory segment
    int shmid = shmget(ShmKey, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    // Attach shared memory to our data space
    sharedData = (SharedData*)shmat(shmid, nullptr, 0);
    if (sharedData == (SharedData*)-1) {
        perror("shmat");
        return 1;
    }

    // Create a named semaphore
    semaphore = sem_open("/my_semaphore", O_CREAT, 0666, 0);
    if (semaphore == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    // Initialize shared data
    sharedData->count = 0;

    // Create Generator process
    pid_t generatorPid = fork();
    if (generatorPid == 0) {
        // Child process (Generator)
        cout << "Generator process is created..." << endl;
        generatorProcess();
        exit(0);
    }

    // Create Monitor process
    pid_t monitorPid = fork();
    if (monitorPid == 0) {
        cout << "Monitor process is created..." << endl;
        // Child process (Monitor)
        monitorProcess();
        exit(0);
    }

    // Wait for both child processes to finish
    waitpid(generatorPid, nullptr, 0);
    waitpid(monitorPid, nullptr, 0);

    // Notify child process to exit
    sem_post(semaphore);

    // Clean up
    sem_close(semaphore);
    sem_unlink("/my_semaphore");
    shmdt(sharedData);
    shmctl(shmid, IPC_RMID, nullptr);

    return 0;
}
