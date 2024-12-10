// C program to demonstrate use of fork() and pipe() 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
  
int main() {
    int pipe1[2]; // Pipe 1: Parent sends data to Child
    int pipe2[2]; // Pipe 2: Child sends data back to Parent

    char child_suffix[] = "howard.edu";
    char parent_suffix[] = "gobison.org";
    char parent_input[100];
    char child_input[100];
    pid_t pid;

    // Create the first pipe
    if (pipe(pipe1) == -1) {
        perror("Error creating first pipe");
        return EXIT_FAILURE;
    }

    // Create the second pipe
    if (pipe(pipe2) == -1) {
        perror("Error creating second pipe");
        return EXIT_FAILURE;
    }

    printf("Enter a string to append: ");
    scanf("%s", parent_input);

    // Fork to create a child process
    pid = fork();

    if (pid < 0) {
        perror("Forking process failed");
        return EXIT_FAILURE;
    } else if (pid > 0) {
        // Parent Process
        close(pipe1[0]); // Close read end of pipe1
        close(pipe2[1]); // Close write end of pipe2

        // Send parent input to child
        write(pipe1[1], parent_input, strlen(parent_input) + 1);
        close(pipe1[1]); // Close write end of pipe1

        // Wait for child process to finish
        wait(NULL);

        // Read modified string from child
        char modified_by_child[200];
        read(pipe2[0], modified_by_child, sizeof(modified_by_child));
        printf("Child appended string: %s\n", modified_by_child);

        // Parent appends its own suffix
        strcat(modified_by_child, parent_suffix);
        printf("Final concatenated string (parent): %s\n", modified_by_child);

        close(pipe2[0]); // Close read end of pipe2
    } else {
        // Child Process
        close(pipe1[1]); // Close write end of pipe1
        close(pipe2[0]); // Close read end of pipe2

        // Read input from parent
        char received_from_parent[100];
        read(pipe1[0], received_from_parent, sizeof(received_from_parent));
        close(pipe1[0]); // Close read end of pipe1

        // Append the child's suffix
        strcat(received_from_parent, child_suffix);
        printf("Child process appended: %s\n", received_from_parent);

        // Send modified string back to parent
        write(pipe2[1], received_from_parent, strlen(received_from_parent) + 1);

        // Request additional input from user
        printf("Child process: Enter another string for parent: ");
        scanf("%s", child_input);

        // Send new input to parent
        write(pipe2[1], child_input, strlen(child_input) + 1);
        close(pipe2[1]); // Close write end of pipe2

        exit(EXIT_SUCCESS);
    }

    return EXIT_SUCCESS;
}