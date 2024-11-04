/* hello_signal.c */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

// Declare a volatile variable to indicate when the signal has been handled
volatile sig_atomic_t flag = 0;

// Signal handler function that prints a message and sets the flag
void handler(int signum)
{
    printf("Hello World!\n");
    flag = 1; // Set the flag to indicate that the signal has been processed
}

int main(int argc, char *argv[])
{
    // Register the `handler` function to handle the SIGALRM signal
    signal(SIGALRM, handler);

    // Infinite loop that repeatedly waits for the signal
    while (1) {
        // Schedule an alarm to go off after 5 seconds
        alarm(5);
        // Reset the flag to prepare for the next signal
        flag = 0;

        // Busy wait until the flag is set by the handler
        while (!flag);

        // Print a message once the flag is set and the signal is handled
        printf("Turing was right!\n");
    }

    // Return 0 (this line is never actually reached)
    return 0;
}
