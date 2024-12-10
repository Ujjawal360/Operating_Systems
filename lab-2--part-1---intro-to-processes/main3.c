#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>

#define MAX_ITERATIONS 30
#define MAX_SLEEP_TIME 10

void create_child_process() {
    // Seed random number generator
    srand(time(NULL) ^ (getpid() << 16));

    // Generate a random number of iterations between 1 and MAX_ITERATIONS
    int iterationCount = rand() % MAX_ITERATIONS + 1;

    for (int i = 0; i < iterationCount; i++) {
        // Child prints PID and that it's going to sleep
        printf("Child Process ID: %d is going to sleep!\n", getpid());

        // Sleep for a random time between 1 and MAX_SLEEP_TIME seconds
        int sleepDuration = rand() % MAX_SLEEP_TIME + 1;
        sleep(sleepDuration);

        // Child wakes up and prints its PID and PPID
        printf("Child Process ID: %d is awake! Where is my Parent: %d?\n", getpid(), getppid());
    }

    // Child process terminates
    exit(0);
}

int main() {
    pid_t childProcess1, childProcess2;
    int status;

    // Fork to create the first child
    childProcess1 = fork();

    if (childProcess1 < 0) {
        // Error occurred during fork
        perror("Fork failed");
        exit(1);
    } else if (childProcess1 == 0) {
        // Child 1 process
        create_child_process();
    } else {
        // Fork to create the second child
        childProcess2 = fork();

        if (childProcess2 < 0) {
            // Error occurred during fork
            perror("Fork failed");
            exit(1);
        } else if (childProcess2 == 0) {
            // Child 2 process
            create_child_process();
        } else {
            // Parent process waits for both children to complete
            pid_t terminatedPid;

            terminatedPid = wait(&status);
            printf("Child Process ID: %d has completed\n", terminatedPid);

            terminatedPid = wait(&status);
            printf("Child Process ID: %d has completed\n", terminatedPid);

            printf("Parent Process (PID: %d) has finished waiting.\n", getpid());
        }
    }
    return 0;
}