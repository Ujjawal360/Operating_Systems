#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

// Declare a volatile flag to indicate when the alarm signal has been handled
volatile sig_atomic_t flag = 0;
// Variable to hold the program's start time
time_t start_time;

// Function to handle the SIGALRM signal
void handle_alarm(int signum)
{
    // Print a message when the alarm goes off
    printf("Hello World!\n");
    // Set the flag to indicate the signal has been processed
    flag = 1;
}

// Function to handle the SIGINT signal (e.g., when Ctrl+C is pressed)
void handle_sigint(int signum)
{
    // Get the current time
    time_t end_time;
    time(&end_time);
    // Print the total duration the program has been running
    printf("\nTotal system time: %ld seconds\n", end_time - start_time);
    // Exit the program
    exit(0);
}

int main(int argc, char *argv[])
{
    // Record the initial time when the program starts
    time(&start_time);

    // Set up signal handlers for SIGALRM and SIGINT
    signal(SIGALRM, handle_alarm);
    signal(SIGINT, handle_sigint);

    // Infinite loop to repeatedly set an alarm and wait for it to trigger
    while (1) {
        // Schedule an alarm to trigger after 1 second
        alarm(1);
        // Reset the flag to wait for the next signal
        flag = 0;

        // Wait in a busy loop until the flag is set by the signal handler
        while (!flag);
    }

    // Return 0 (though this part is never reached)
    return 0;
}
