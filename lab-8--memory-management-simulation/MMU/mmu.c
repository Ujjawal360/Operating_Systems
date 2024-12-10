#include <limits.h>  // For INT_MAX
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "list.h"
#include "util.h"

// Converts a string to uppercase
void convert_to_uppercase(char *str) {
    for (int i = 0; i < strlen(str); i++) {
        str[i] = toupper(str[i]);
    }
}

// Processes input arguments and parses input data
void process_input(char *args[], int input[][2], int *num_requests, int *partition_size, int *policy) {
    FILE *input_file = fopen(args[1], "r");
    if (!input_file) {
        fprintf(stderr, "Error: Invalid file path\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    parse_file(input_file, input, num_requests, partition_size);
    fclose(input_file);

    convert_to_uppercase(args[2]);

    if (strcmp(args[2], "-F") == 0 || strcmp(args[2], "-FIFO") == 0) {
        *policy = 1;  // FIFO policy
    } else if (strcmp(args[2], "-B") == 0 || strcmp(args[2], "-BESTFIT") == 0) {
        *policy = 2;  // Best Fit policy
    } else if (strcmp(args[2], "-W") == 0 || strcmp(args[2], "-WORSTFIT") == 0) {
        *policy = 3;  // Worst Fit policy
    } else {
        printf("Usage: ./mmu <input file> -{F | B | W}\n(F=FIFO | B=BESTFIT | W=WORSTFIT)\n");
        exit(EXIT_FAILURE);
    }
}

// Allocates memory based on the given policy
void allocate_block(list_t *free_blocks, list_t *allocated_blocks, int pid, int block_size, int policy) {
    node_t *current_node = free_blocks->head;
    node_t *best_fit = NULL;
    block_t *selected_block = NULL;

    // Select block based on allocation policy
    if (policy == 1) { // FIFO
        while (current_node) {
            int size = current_node->blk->end - current_node->blk->start + 1;
            if (size >= block_size) {
                best_fit = current_node;
                break;
            }
            current_node = current_node->next;
        }
    } else if (policy == 2) { // Best Fit
        int min_size = INT_MAX;
        while (current_node) {
            int size = current_node->blk->end - current_node->blk->start + 1;
            if (size >= block_size && size < min_size) {
                min_size = size;
                best_fit = current_node;
            }
            current_node = current_node->next;
        }
    } else if (policy == 3) { // Worst Fit
        int max_size = -1;
        while (current_node) {
            int size = current_node->blk->end - current_node->blk->start + 1;
            if (size >= block_size && size > max_size) {
                max_size = size;
                best_fit = current_node;
            }
            current_node = current_node->next;
        }
    }

    if (!best_fit) {
        printf("Error: Insufficient memory\n");
        return;
    }

    // Allocate selected block
    selected_block = best_fit->blk;
    int original_end = selected_block->end;
    selected_block->pid = pid;
    selected_block->end = selected_block->start + block_size - 1;

    // Add allocated block to the list
    list_add_ascending_by_address(allocated_blocks, selected_block);

    // Handle fragmentation
    if (selected_block->end < original_end) {
        block_t *fragment = malloc(sizeof(block_t));
        fragment->pid = 0;  // Mark as free
        fragment->start = selected_block->end + 1;
        fragment->end = original_end;

        if (policy == 1) {
            list_add_to_back(free_blocks, fragment);
        } else if (policy == 2) {
            list_add_ascending_by_blocksize(free_blocks, fragment);
        } else if (policy == 3) {
            list_add_descending_by_blocksize(free_blocks, fragment);
        }
    }

    // Remove the allocated block from the freelist
    list_remove_from_front(free_blocks);
}

// Deallocates memory associated with a PID
void free_memory(list_t *allocated_blocks, list_t *free_blocks, int pid, int policy) {
    node_t *current_node = allocated_blocks->head;
    node_t *previous_node = NULL;
    block_t *block_to_free = NULL;

    // Locate the block to deallocate
    while (current_node) {
        if (current_node->blk->pid == pid) {
            block_to_free = current_node->blk;
            if (!previous_node) {
                allocated_blocks->head = current_node->next;
            } else {
                previous_node->next = current_node->next;
            }
            free(current_node);
            break;
        }
        previous_node = current_node;
        current_node = current_node->next;
    }

    if (!block_to_free) {
        printf("Error: Unable to locate memory for PID: %d\n", pid);
        return;
    }

    // Mark block as free and return to freelist
    block_to_free->pid = 0;
    list_add_ascending_by_address(free_blocks, block_to_free);
}

// Merges adjacent free blocks
list_t *merge_memory(list_t *list) {
    list_t *temp_list = list_alloc();
    block_t *current_block;

    while ((current_block = list_remove_from_front(list))) {
        list_add_ascending_by_address(temp_list, current_block);
    }

    list_coalese_nodes(temp_list);
    return temp_list;
}

// Prints the state of the memory list
void display_memory(list_t *list, const char *message) {
    node_t *current_node = list->head;
    block_t *current_block;
    int index = 0;

    printf("%s:\n", message);
    while (current_node) {
        current_block = current_node->blk;
        printf("Block %d:\tSTART: %d\tEND: %d", index, current_block->start, current_block->end);

        if (current_block->pid != 0)
            printf("\tPID: %d\n", current_block->pid);
        else
            printf("\n");

        current_node = current_node->next;
        index++;
    }
}

// Main function
int main(int argc, char *argv[]) {
    int partition_size, input_data[200][2], request_count = 0, policy;

    list_t *free_blocks = list_alloc();
    list_t *allocated_blocks = list_alloc();

    if (argc != 3) {
        printf("Usage: ./mmu <input file> -{F | B | W}\n(F=FIFO | B=BESTFIT | W=WORSTFIT)\n");
        exit(EXIT_FAILURE);
    }

    process_input(argv, input_data, &request_count, &partition_size, &policy);

    // Initialize the memory partition
    block_t *initial_partition = malloc(sizeof(block_t));
    initial_partition->start = 0;
    initial_partition->end = partition_size - 1;

    list_add_to_front(free_blocks, initial_partition);

    // Simulate memory operations
    for (int i = 0; i < request_count; i++) {
        printf("************************\n");
        if (input_data[i][0] > 0) {
            printf("ALLOCATE: %d FOR PID: %d\n", input_data[i][1], input_data[i][0]);
            allocate_block(free_blocks, allocated_blocks, input_data[i][0], input_data[i][1], policy);
        } else if (input_data[i][0] < 0) {
            printf("DEALLOCATE MEMORY FOR PID: %d\n", abs(input_data[i][0]));
            free_memory(allocated_blocks, free_blocks, abs(input_data[i][0]), policy);
        } else {
            printf("COALESCE MEMORY\n");
            free_blocks = merge_memory(free_blocks);
        }
        printf("************************\n");
        display_memory(free_blocks, "Free Memory");
        display_memory(allocated_blocks, "\nAllocated Memory");
        printf("\n");
    }

    list_free(free_blocks);
    list_free(allocated_blocks);
    return 0;
}