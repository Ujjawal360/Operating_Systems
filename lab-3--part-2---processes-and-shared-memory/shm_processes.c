#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

// Function prototype for client process
void ExecuteClientProcess(int []);

int main(int argc, char *argv[]) {
    int sharedMemoryID;
    int *sharedMemoryPointer;
    pid_t childProcessID;
    int childStatus;

    if (argc != 5) {
        printf("Usage: %s num1 num2 num3 num4\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Create shared memory for storing four integers
    sharedMemoryID = shmget(IPC_PRIVATE, 4 * sizeof(int), IPC_CREAT | 0666);
    if (sharedMemoryID < 0) {
        perror("Error: Failed to create shared memory (server)");
        exit(EXIT_FAILURE);
    }
    printf("Server has created shared memory for four integers...\n");

    // Attach shared memory to the server's address space
    sharedMemoryPointer = (int *)shmat(sharedMemoryID, NULL, 0);
    if (*sharedMemoryPointer == -1) {
        perror("Error: Failed to attach shared memory (server)");
        exit(EXIT_FAILURE);
    }
    printf("Server has successfully attached the shared memory...\n");

    // Populate shared memory with input values
    sharedMemoryPointer[0] = atoi(argv[1]);
    sharedMemoryPointer[1] = atoi(argv[2]);
    sharedMemoryPointer[2] = atoi(argv[3]);
    sharedMemoryPointer[3] = atoi(argv[4]);
    printf("Server has stored the values %d %d %d %d in shared memory...\n",
           sharedMemoryPointer[0], sharedMemoryPointer[1], sharedMemoryPointer[2], sharedMemoryPointer[3]);

    printf("Server is preparing to fork a child process...\n");
    childProcessID = fork();

    if (childProcessID < 0) {
        perror("Error: Forking child process failed (server)");
        exit(EXIT_FAILURE);
    } else if (childProcessID == 0) {
        // In child process, execute client functionality
        ExecuteClientProcess(sharedMemoryPointer);
        exit(EXIT_SUCCESS);
    }

    // Parent process waits for child to finish execution
    wait(&childStatus);
    printf("Server detected the child process has completed...\n");

    // Detach and clean up shared memory
    shmdt((void *)sharedMemoryPointer);
    printf("Server has detached the shared memory...\n");
    shmctl(sharedMemoryID, IPC_RMID, NULL);
    printf("Server has successfully removed the shared memory...\n");

    printf("Server process terminating...\n");
    exit(EXIT_SUCCESS);
}

void ExecuteClientProcess(int sharedMemory[]) {
    printf("   Client process initiated\n");
    printf("   Client reads values %d %d %d %d from shared memory\n",
           sharedMemory[0], sharedMemory[1], sharedMemory[2], sharedMemory[3]);
    printf("   Client process is terminating\n");
}
