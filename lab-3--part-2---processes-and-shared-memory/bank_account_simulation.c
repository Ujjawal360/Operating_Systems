#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define SHARED_MEMORY_KEY 9876 // Key for the shared memory segment

void RunParentProcess(int *Balance, int *ActiveTurn); // Function for Parent (Dad)
void RunChildProcess(int *Balance, int *ActiveTurn);  // Function for Child (Student)

int main() {
    int sharedMemoryID;
    int *sharedMemoryPointer;
    pid_t childProcessID;
    int childExitStatus;

    // Create a shared memory segment to hold two integers
    sharedMemoryID = shmget(SHARED_MEMORY_KEY, 2 * sizeof(int), IPC_CREAT | 0666);
    if (sharedMemoryID < 0) {
        fprintf(stderr, "Error: Unable to create shared memory.\n");
        exit(EXIT_FAILURE);
    }
    printf("Shared memory segment successfully created.\n");

    // Attach the shared memory segment to this process
    sharedMemoryPointer = (int *)shmat(sharedMemoryID, NULL, 0);
    if (sharedMemoryPointer == (int *)-1) {
        fprintf(stderr, "Error: Failed to attach shared memory.\n");
        exit(EXIT_FAILURE);
    }
    printf("Shared memory attached successfully.\n");

    // Initialize the shared variables
    sharedMemoryPointer[0] = 0; // Balance starts at 0
    sharedMemoryPointer[1] = 0; // Initial turn is set to 0 (Parent's turn)

    // Create a child process
    childProcessID = fork();
    if (childProcessID < 0) {
        fprintf(stderr, "Error: Forking process failed.\n");
        exit(EXIT_FAILURE);
    }

    if (childProcessID == 0) {
        // Child process execution
        RunChildProcess(&sharedMemoryPointer[0], &sharedMemoryPointer[1]);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process execution
        RunParentProcess(&sharedMemoryPointer[0], &sharedMemoryPointer[1]);

        // Wait for the child process to complete
        wait(&childExitStatus);

        // Clean up shared memory resources
        shmdt(sharedMemoryPointer);
        shmctl(sharedMemoryID, IPC_RMID, NULL);
        printf("Shared memory detached and deleted successfully.\n");
    }

    return 0;
}

void RunParentProcess(int *Balance, int *ActiveTurn) {
    int currentBalance, depositAmount;
    srand(time(NULL)); // Seed random number generator for Parent

    for (int i = 0; i < 25; i++) {
        sleep(rand() % 6); // Simulate random delay between 0-5 seconds

        currentBalance = *Balance;
        while (*ActiveTurn != 0); // Wait for Parent's turn

        // Check if the account balance is $100 or less
        if (currentBalance <= 100) {
            depositAmount = rand() % 101; // Random deposit between $0 and $100

            if (depositAmount % 2 == 0) { // Even-numbered deposit
                currentBalance += depositAmount;
                printf("Dad: Deposited $%d / Current Balance = $%d\n", depositAmount, currentBalance);
            } else {
                printf("Dad: No money to deposit this time.\n");
            }
        } else {
            printf("Dad: Student has sufficient cash ($%d).\n", currentBalance);
        }

        // Update shared memory and pass turn to Child
        *Balance = currentBalance;
        *ActiveTurn = 1;
    }
}

void RunChildProcess(int *Balance, int *ActiveTurn) {
    int currentBalance, withdrawalAmount;
    srand(time(NULL) + 1); // Seed random number generator for Child

    for (int i = 0; i < 25; i++) {
        sleep(rand() % 6); // Simulate random delay between 0-5 seconds

        currentBalance = *Balance;
        while (*ActiveTurn != 1); // Wait for Child's turn

        withdrawalAmount = rand() % 51; // Random withdrawal between $0 and $50
        printf("Student: Needs $%d.\n", withdrawalAmount);

        if (withdrawalAmount <= currentBalance) {
            currentBalance -= withdrawalAmount;
            printf("Student: Withdrew $%d / Remaining Balance = $%d\n", withdrawalAmount, currentBalance);
        } else {
            printf("Student: Insufficient funds ($%d).\n", currentBalance);
        }

        // Update shared memory and pass turn back to Parent
        *Balance = currentBalance;
        *ActiveTurn = 0;
    }
}