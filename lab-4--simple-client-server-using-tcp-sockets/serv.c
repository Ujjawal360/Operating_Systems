#include <netinet/in.h>  // For socket address structures
#include <stdio.h>       // For standard I/O functions
#include <string.h>      // For string manipulation
#include <stdlib.h>      // For general utility functions like atoi()
#include <sys/socket.h>  // For socket operations
#include <sys/types.h>   // For socket types
#include <unistd.h>      // For close()

#include "list.h"  // Assumed to contain linked list operations

#define PORT 9001            // Port to listen on
#define ACK_MESSAGE "ACK"    // Acknowledgment message

int main(int argc, char const *argv[]) {
    // Create the server socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    char inputBuffer[1024];   // Buffer for receiving commands
    char responseBuffer[1024]; // Buffer for sending responses
    char *command;           // Pointer to command tokens
    int bytesRead, value, index;

    // Initialize the server address structure
    struct sockaddr_in serverAddress;
    list_t *linkedList = list_alloc();  // Create an empty linked list

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the specified port
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 1) < 0) {
        perror("Failed to listen on socket");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for a client connection...\n");

    // Accept a client connection
    int clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket < 0) {
        perror("Failed to accept client connection");
        exit(EXIT_FAILURE);
    }
    printf("Client connected successfully.\n");

    // Main loop to handle client commands
    while (1) {
        memset(inputBuffer, 0, sizeof(inputBuffer)); // Clear input buffer

        // Receive command from the client
        bytesRead = recv(clientSocket, inputBuffer, sizeof(inputBuffer), 0);
        if (bytesRead <= 0) continue; // Skip empty or invalid reads
        inputBuffer[bytesRead] = '\0'; // Ensure null-terminated string

        command = strtok(inputBuffer, " "); // Extract the command
        memset(responseBuffer, 0, sizeof(responseBuffer)); // Clear response buffer

        // Handle different commands
        if (strcmp(command, "exit") == 0) {
            list_free(linkedList); // Free the linked list
            printf("Shutting down the server...\n");
            close(clientSocket);
            close(serverSocket);
            exit(EXIT_SUCCESS);
        } else if (strcmp(command, "print") == 0) {
            snprintf(responseBuffer, sizeof(responseBuffer), "%s", listToString(linkedList));
        } else if (strcmp(command, "get_length") == 0) {
            value = list_length(linkedList);
            snprintf(responseBuffer, sizeof(responseBuffer), "Length = %d", value);
        } else if (strcmp(command, "add_back") == 0) {
            value = atoi(strtok(NULL, " "));
            list_add_to_back(linkedList, value);
            snprintf(responseBuffer, sizeof(responseBuffer), "%s %d", ACK_MESSAGE, value);
        } else if (strcmp(command, "add_front") == 0) {
            value = atoi(strtok(NULL, " "));
            list_add_to_front(linkedList, value);
            snprintf(responseBuffer, sizeof(responseBuffer), "%s %d", ACK_MESSAGE, value);
        } else if (strcmp(command, "add_position") == 0) {
            index = atoi(strtok(NULL, " "));
            value = atoi(strtok(NULL, " "));
            list_add_at_index(linkedList, index, value);
            snprintf(responseBuffer, sizeof(responseBuffer), "%s %d at %d", ACK_MESSAGE, value, index);
        } else if (strcmp(command, "remove_back") == 0) {
            value = list_remove_from_back(linkedList);
            snprintf(responseBuffer, sizeof(responseBuffer), "Removed = %d", value);
        } else if (strcmp(command, "remove_front") == 0) {
            value = list_remove_from_front(linkedList);
            snprintf(responseBuffer, sizeof(responseBuffer), "Removed = %d", value);
        } else if (strcmp(command, "remove_position") == 0) {
            index = atoi(strtok(NULL, " "));
            value = list_remove_at_index(linkedList, index);
            snprintf(responseBuffer, sizeof(responseBuffer), "Removed = %d from %d", value, index);
        } else if (strcmp(command, "get") == 0) {
            index = atoi(strtok(NULL, " "));
            value = list_get_elem_at(linkedList, index);
            snprintf(responseBuffer, sizeof(responseBuffer), "Element at %d = %d", index, value);
        } else {
            snprintf(responseBuffer, sizeof(responseBuffer), "Unknown command");
        }

        // Send the response back to the client
        send(clientSocket, responseBuffer, strlen(responseBuffer) + 1, 0);
    }

    return 0;
}
