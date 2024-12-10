#include <netinet/in.h>  // For network-related structures like sockaddr_in
#include <stdio.h>       // For standard input/output functions
#include <stdlib.h>      // For general utilities like exit(), atoi()
#include <string.h>      // For string operations like strlen, strcmp
#include <sys/socket.h>  // For socket operations
#include <sys/types.h>   // For data types used in sockets
#include <unistd.h>      // For close() function

#define PORT 9001                   // Server port number
#define MAX_INPUT_LENGTH 1024       // Maximum input length for commands

// Function to read a command from the user
char* readCommand(char *inputBuffer) {
    do {
        if (fgets(inputBuffer, MAX_INPUT_LENGTH, stdin) == NULL && ferror(stdin)) {
            fprintf(stderr, "Error reading input");
            exit(1);
        }
    } while (inputBuffer[0] == '\n'); // Skip empty lines

    inputBuffer[strlen(inputBuffer) - 1] = '\0'; // Remove newline character
    return inputBuffer;
}

int main(int argc, char const *argv[]) {
    // Create a TCP socket
    int socketID = socket(AF_INET, SOCK_STREAM, 0);
    if (socketID < 0) {
        perror("Failed to create socket");
        exit(1);
    }

    char inputBuffer[MAX_INPUT_LENGTH]; // Buffer for user input
    char serverResponse[MAX_INPUT_LENGTH]; // Buffer for server responses
    struct sockaddr_in serverAddress;

    // Set up server address structure
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Attempt to connect to the server
    if (connect(socketID, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Failed to connect to the server");
        exit(1);
    }
    printf("Successfully connected to the server.\n");

    // Main loop for client operations
    while (1) {
        printf("Enter a command (or 'menu' for options): ");
        readCommand(inputBuffer);

        // Send the command to the server
        send(socketID, inputBuffer, strlen(inputBuffer), 0);

        // Handle the "exit" command to terminate the client
        if (strcmp(inputBuffer, "exit") == 0) {
            printf("Closing the client...\n");
            close(socketID);
            exit(0);
        } 
        // Display available commands if "menu" is entered
        else if (strcmp(inputBuffer, "menu") == 0) {
            printf("AVAILABLE COMMANDS:\n");
            printf("-------------------\n");
            printf("1. print\n2. get_length\n3. add_back <value>\n4. add_front <value>\n5. add_position <index> <value>\n");
            printf("6. remove_back\n7. remove_front\n8. remove_position <index>\n9. get <index>\n10. exit\n");
        }

        // Receive the server's response
        recv(socketID, serverResponse, sizeof(serverResponse), 0);
        printf("\nSERVER RESPONSE: %s\n", serverResponse);

        // Clear the input buffer for the next command
        memset(inputBuffer, 0, MAX_INPUT_LENGTH);
    }

    return 0;
}