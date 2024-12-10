#include <stdio.h>
#include <stdlib.h>

// Function declarations
int add(int a, int b);
int subtract(int a, int b);
int multiply(int a, int b);
int divide(int a, int b);
void exit_program();

int main(void)
{
    int a = 6, b = 3;  // Initialized operands
    char choice;

    // Array to store function pointers for operations
    int (*operations[])(int, int) = { add, subtract, multiply, divide };
    void (*exit_func)() = exit_program;

    // Display the operands
    printf("Operand 'a': %d | Operand 'b': %d\n", a, b);
    printf("Choose an operation (0: Add | 1: Subtract | 2: Multiply | 3: Divide | 4: Exit): ");

    // Read user input
    scanf(" %c", &choice);

    // Process based on user input
    if (choice >= '0' && choice <= '3') {
        int result = operations[choice - '0'](a, b);  // Execute selected operation
        printf("Result: %d\n", result);
    } else if (choice == '4') {
        exit_func();  // Call the exit function
    }

    return 0;
}

// Function definitions
int add(int a, int b) {
    printf("Performing addition...\n");
    return a + b;
}

int subtract(int a, int b) {
    printf("Performing subtraction...\n");
    return a - b;
}

int multiply(int a, int b) {
    printf("Performing multiplication...\n");
    return a * b;
}

int divide(int a, int b) {
    if (b == 0) {
        printf("Error: Cannot divide by zero.\n");
        return 0;
    }
    printf("Performing division...\n");
    return a / b;
}

void exit_program() {
    printf("Terminating the program...\n");
    exit(0);
}
