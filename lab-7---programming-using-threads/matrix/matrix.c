#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX 20

// Matrices and their resulting computations
int matA[MAX][MAX];
int matB[MAX][MAX];
int matSumResult[MAX][MAX];
int matDiffResult[MAX][MAX];
int matProductResult[MAX][MAX];

// Struct to define the row and column of a matrix cell
typedef struct {
    int row;
    int col;
} MatrixCell;

// Function to populate a matrix with random values between 1 and 10
void populateMatrix(int matrix[MAX][MAX]) {
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) {
            matrix[i][j] = rand() % 10 + 1;
        }
    }
}

// Function to display a matrix
void displayMatrix(int matrix[MAX][MAX]) {
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) {
            printf("%5d", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Thread function to compute the sum of two matrices
void* calculateSum(void* args) {
    MatrixCell* cell = (MatrixCell*)args;
    matSumResult[cell->row][cell->col] = matA[cell->row][cell->col] + matB[cell->row][cell->col];
    free(cell);
    pthread_exit(0);
}

// Thread function to compute the difference between two matrices
void* calculateDifference(void* args) {
    MatrixCell* cell = (MatrixCell*)args;
    matDiffResult[cell->row][cell->col] = matA[cell->row][cell->col] - matB[cell->row][cell->col];
    free(cell);
    pthread_exit(0);
}

// Thread function to compute the product of two matrices
void* calculateProduct(void* args) {
    MatrixCell* cell = (MatrixCell*)args;
    matProductResult[cell->row][cell->col] = 0;
    for (int k = 0; k < MAX; k++) {
        matProductResult[cell->row][cell->col] += matA[cell->row][k] * matB[k][cell->col];
    }
    free(cell);
    pthread_exit(0);
}

int main() {
    srand(time(0));

    // Populate matrices with random values
    populateMatrix(matA);
    populateMatrix(matB);

    // Display the original matrices
    printf("Matrix A:\n");
    displayMatrix(matA);
    printf("Matrix B:\n");
    displayMatrix(matB);

    pthread_t threads[MAX * MAX * 3]; // Threads for sum, difference, and product
    int threadIndex = 0;

    // Create threads for each computation type (sum, difference, product)
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) {
            MatrixCell* sumCell = (MatrixCell*)malloc(sizeof(MatrixCell));
            MatrixCell* diffCell = (MatrixCell*)malloc(sizeof(MatrixCell));
            MatrixCell* prodCell = (MatrixCell*)malloc(sizeof(MatrixCell));

            sumCell->row = i;
            sumCell->col = j;
            diffCell->row = i;
            diffCell->col = j;
            prodCell->row = i;
            prodCell->col = j;

            pthread_create(&threads[threadIndex++], NULL, calculateSum, sumCell);
            pthread_create(&threads[threadIndex++], NULL, calculateDifference, diffCell);
            pthread_create(&threads[threadIndex++], NULL, calculateProduct, prodCell);
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < threadIndex; i++) {
        pthread_join(threads[i], NULL);
    }

    // Display the results
    printf("Sum Result:\n");
    displayMatrix(matSumResult);
    printf("Difference Result:\n");
    displayMatrix(matDiffResult);
    printf("Product Result:\n");
    displayMatrix(matProductResult);

    return 0;
}