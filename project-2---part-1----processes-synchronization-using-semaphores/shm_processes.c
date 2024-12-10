#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

void  clientHandler(int sharedMemory[]);

int  main(int argc, char *argv[])
{
    int sharedMemoryID;
    int *sharedMemoryPointer;
    pid_t processID;
    int status;

    if (argc != 5) {
        printf("Usage: %s #1 #2 #3 #4\n", argv[0]);
        exit(1);
    }

    sharedMemoryID = shmget(IPC_PRIVATE, 4 * sizeof(int), IPC_CREAT | 0666);
    if (sharedMemoryID < 0) {
        printf("Error creating shared memory (server)\n");
        exit(1);
    }
    printf("Server has created shared memory for four integers...\n");

    sharedMemoryPointer = (int *) shmat(sharedMemoryID, NULL, 0);
    if (*sharedMemoryPointer == -1) {
        printf("Error attaching shared memory (server)\n");
        exit(1);
    }
    printf("Server has attached shared memory...\n");

    sharedMemoryPointer[0] = atoi(argv[1]);
    sharedMemoryPointer[1] = atoi(argv[2]);
    sharedMemoryPointer[2] = atoi(argv[3]);
    sharedMemoryPointer[3] = atoi(argv[4]);
    printf("Server has stored %d %d %d %d in shared memory...\n", 
           sharedMemoryPointer[0], sharedMemoryPointer[1], sharedMemoryPointer[2], sharedMemoryPointer[3]);

    printf("Server is forking a child process...\n");
    processID = fork();
    if (processID < 0) {
        printf("Fork failed (server)\n");
        exit(1);
    }
    else if (processID == 0) {
        clientHandler(sharedMemoryPointer);
        exit(0);
    }

    wait(&status);
    printf("Server has confirmed the completion of the child process...\n");
    shmdt((void *) sharedMemoryPointer);
    printf("Server has detached shared memory...\n");
    shmctl(sharedMemoryID, IPC_RMID, NULL);
    printf("Server has removed shared memory...\n");
    printf("Server exits...\n");
    exit(0);
}

void clientHandler(int sharedMemory[])
{
    printf("   Client process has started\n");
    printf("   Client found %d %d %d %d in shared memory\n", 
           sharedMemory[0], sharedMemory[1], sharedMemory[2], sharedMemory[3]);
    printf("   Client process is exiting\n");
}
