#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <semaphore.h>
#include "header.h"

using namespace std;

// Shared memory key
#define SHM_KEY 1234
// Number of threads
#define NUM_THREADS 5


// Global variables
SharedData* sharedData;
sem_t* semaphore;

// Signal handler for SIGUSR1
void signalHandler(int signum) {
    
}

// Generator process function
void generatorProcess() {
    while (true) {
        // Generate a random value and update shared memory
        sharedData->value++;

        cout << sharedData->value << " inside generator" << endl;

        // Post to the semaphore to notify the monitor process
        sem_post(semaphore);

        // Wait for the semaphore to be released by the monitor
        sem_wait(semaphore);

        sleep(2);
    }
}

// Monitor process function
void monitorProcess() {
    while (true) {
        // Wait for the semaphore to be posted by the generator
        sem_wait(semaphore);

        // Print the current shared value
        std::cout << "Monitor Process - Shared Value: " << sharedData->value << std::endl;

        // Post to the semaphore to notify the generator
        sem_post(semaphore);
    }
}

int main() {
    // Create or get a shared memory segment
    int shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
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
    sharedData->value = 0;

    // Set up signal handlers
    signal(SIGUSR1, signalHandler);

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

    // Detach shared memory
    shmdt(sharedData);

    // Close and unlink the semaphore
    sem_close(semaphore);
    sem_unlink("/my_semaphore");

    // Remove the shared memory segment
    shmctl(shmid, IPC_RMID, nullptr);

    return 0;
}
