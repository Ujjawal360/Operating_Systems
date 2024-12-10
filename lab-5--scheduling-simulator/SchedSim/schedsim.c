// C program to simulate different CPU scheduling algorithms
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "process.h"
#include "util.h"

// Function to compute waiting times for processes using Round Robin scheduling
void calculateWaitingTimeRR(ProcessType processes[], int count, int quantum) {
    int remainingBurstTime[count];
    for (int i = 0; i < count; i++) {
        remainingBurstTime[i] = processes[i].bt;
    }

    int currentTime = 0;
    while (1) {
        int allDone = 1;

        for (int i = 0; i < count; i++) {
            if (remainingBurstTime[i] > 0) {
                allDone = 0;
                if (remainingBurstTime[i] > quantum) {
                    currentTime += quantum;
                    remainingBurstTime[i] -= quantum;
                } else {
                    currentTime += remainingBurstTime[i];
                    processes[i].wt = currentTime - processes[i].bt;
                    remainingBurstTime[i] = 0;
                }
            }
        }

        if (allDone) break;
    }
}

// Function to compute waiting times using Shortest Job First scheduling
void calculateWaitingTimeSJF(ProcessType processes[], int count) {
    int remainingBurstTime[count];
    for (int i = 0; i < count; i++) {
        remainingBurstTime[i] = processes[i].bt;
    }

    int completed = 0, currentTime = 0;

    while (completed < count) {
        int shortest = INT_MAX, index = -1;

        for (int i = 0; i < count; i++) {
            if (remainingBurstTime[i] > 0 && remainingBurstTime[i] < shortest && processes[i].art <= currentTime) {
                shortest = remainingBurstTime[i];
                index = i;
            }
        }

        remainingBurstTime[index]--;
        currentTime++;

        if (remainingBurstTime[index] == 0) {
            completed++;
            processes[index].wt = currentTime - processes[index].art - processes[index].bt;
        }
    }
}

// Function to compute waiting times using First Come First Serve scheduling
void calculateWaitingTimeFCFS(ProcessType processes[], int count) {
    processes[0].wt = processes[0].art;

    for (int i = 1; i < count; i++) {
        processes[i].wt = processes[i - 1].bt + processes[i - 1].wt;
    }
}

// Function to calculate turnaround times
void calculateTurnAroundTime(ProcessType processes[], int count) {
    for (int i = 0; i < count; i++) {
        processes[i].tat = processes[i].bt + processes[i].wt;
    }
}

// Comparator to sort processes by priority
int comparePriority(const void *a, const void *b) {
    ProcessType *p1 = (ProcessType *)a;
    ProcessType *p2 = (ProcessType *)b;

    return (p2->pri - p1->pri);
}

// Function to simulate Priority scheduling
void simulatePriority(ProcessType processes[], int count) {
    qsort(processes, count, sizeof(ProcessType), comparePriority);
    calculateWaitingTimeFCFS(processes, count);
    calculateTurnAroundTime(processes, count);

    printf("\nPriority Scheduling:\n");
}

// Print scheduling metrics
void printMetrics(ProcessType processes[], int count) {
    int totalWaiting = 0, totalTurnAround = 0;

    printf("Process\tBurst Time\tWaiting Time\tTurnaround Time\n");
    for (int i = 0; i < count; i++) {
        totalWaiting += processes[i].wt;
        totalTurnAround += processes[i].tat;
        printf("%d\t%d\t\t%d\t\t%d\n", processes[i].pid, processes[i].bt, processes[i].wt, processes[i].tat);
    }

    printf("Average Waiting Time: %.2f\n", (float)totalWaiting / count);
    printf("Average Turnaround Time: %.2f\n", (float)totalTurnAround / count);
}

// Initialize process list from a file
ProcessType *initializeProcesses(const char *filename, int *count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Invalid file path\n");
        exit(1);
    }

    ProcessType *processes = parse_file(file, count);
    fclose(file);
    return processes;
}

// Main function
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input-file-path>\n", argv[0]);
        return 1;
    }

    int count;
    ProcessType *processes;

    // FCFS
    processes = initializeProcesses(argv[1], &count);
    calculateWaitingTimeFCFS(processes, count);
    calculateTurnAroundTime(processes, count);
    printf("\nFirst Come First Serve Scheduling:\n");
    printMetrics(processes, count);

    // SJF
    processes = initializeProcesses(argv[1], &count);
    calculateWaitingTimeSJF(processes, count);
    calculateTurnAroundTime(processes, count);
    printf("\nShortest Job First Scheduling:\n");
    printMetrics(processes, count);

    // Priority
    processes = initializeProcesses(argv[1], &count);
    simulatePriority(processes, count);
    printMetrics(processes, count);

    // Round Robin
    processes = initializeProcesses(argv[1], &count);
    int quantum = 2;
    calculateWaitingTimeRR(processes, count, quantum);
    calculateTurnAroundTime(processes, count);
    printf("\nRound Robin Scheduling (Quantum = %d):\n", quantum);
    printMetrics(processes, count);

    return 0;
}
