#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "process.h"
#include "util.h"

#define DEBUG 0  // Set to 1 for detailed output

/**
 * Function pointer type definition for custom comparisons.
 * Implementers must cast inputs to their actual types
 * and provide logic for comparing them.
 */
typedef int (*Comparer) (const void *a, const void *b);

/**
 * Custom comparator for comparing two processes.
 * Assumptions:
 * - Process IDs are unique.
 * - No two processes share the same arrival time.
 */
int process_comparer(const void *a, const void *b)
{
    Process *p1 = (Process *)a;
    Process *p2 = (Process *)b;

    // First, prioritize based on descending order of priority
    if (p1->priority != p2->priority) {
        return p2->priority - p1->priority;
    }

    // If priorities are the same, compare arrival times (ascending order)
    if (p1->arrival_time != p2->arrival_time) {
        return p1->arrival_time - p2->arrival_time;
    }

    // If both priority and arrival time are the same, sort by PID (ascending order)
    return p1->pid - p2->pid;
}

int main(int argc, char *argv[])
{
    // Validate command-line arguments
    if (argc < 2) {
        fprintf(stderr, "Usage: ./func-ptr <input-file-path>\n");
        fflush(stdout);
        return 1;
    }

    // Parse the input file
    FILE *input_file = fopen(argv[1], "r");
    if (!input_file) {
        fprintf(stderr, "Error: Invalid file path\n");
        fflush(stdout);
        return 1;
    }

    Process *processes = parse_file(input_file);

    // Sort the processes using the custom comparator
    Comparer comparer = &process_comparer;

#if DEBUG
    for (int i = 0; i < P_SIZE; i++) {
        printf("%d (%d, %d) ",
               processes[i].pid,
               processes[i].priority, 
               processes[i].arrival_time);
    }
    printf("\n");
#endif

    qsort(processes, P_SIZE, sizeof(Process), comparer);

    // Print the sorted process data
    for (int i = 0; i < P_SIZE; i++) {
        printf("%d (%d, %d)\n",
               processes[i].pid,
               processes[i].priority, 
               processes[i].arrival_time);
    }

    fflush(stdout);
    fflush(stderr);

    // Clean up memory and close file
    free(processes);
    fclose(input_file);
    return 0;
}