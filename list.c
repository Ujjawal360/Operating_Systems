// list/list.c
// 
// Linked list implementation
// 
// <Your Name>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

/* Create a new node with given value */
node_t *createNode(elem value) {
    node_t *newNode = malloc(sizeof(node_t));
    if (newNode == NULL) {
        // Handle memory allocation failure
        return NULL;
    }
    newNode->value = value;
    newNode->next = NULL;
    return newNode;
}

/* Create and initialize a new list */
list_t *createList() { 
    list_t *newList = malloc(sizeof(list_t)); 
    if (newList == NULL) return NULL;
    newList->head = NULL;
    return newList;
}

/* Delete all nodes and reset the list */
void deleteList(list_t *list) {
    if (list == NULL) return;
    
    node_t *current = list->head;
    while (current != NULL) {
        node_t *temp = current;
        current = current->next;
        free(temp);
    }
    list->head = NULL; // Mark the list as empty
}

/* Display the list contents */
void displayList(list_t *list) {
    if (list == NULL || list->head == NULL) {
        printf("NULL\n");
        return;
    }

    node_t *current = list->head;
    while (current != NULL) {
        printf("%d->", current->value);
        current = current->next;
    }
    printf("NULL\n");
}

/* Convert list to string representation */
char *listToString(list_t *list) {
    if (list == NULL || list->head == NULL) {
        char *emptyStr = malloc(6); // "NULL\0"
        if (emptyStr == NULL) return NULL; // Handle allocation failure
        strcpy(emptyStr, "NULL");
        return emptyStr;
    }

    // Calculate buffer size: each int (max 11 chars) + "->" (2 chars) + "NULL" (4 chars) + null terminator
    int maxNodes = getListLength(list);
    int bufSize = (13 * maxNodes) + 5;
    char *buffer = malloc(sizeof(char) * bufSize);
    if (buffer == NULL) return NULL; // Handle allocation failure
    buffer[0] = '\0'; // Initialize buffer

    node_t *current = list->head;
    while (current != NULL) {
        char temp[20];
        snprintf(temp, sizeof(temp), "%d->", current->value);
        strcat(buffer, temp);
        current = current->next;
    }
    strcat(buffer, "NULL");
    return buffer;
}

/* Get the number of elements in the list */
int getListLength(list_t *list) { 
    if (list == NULL) return 0;

    int count = 0;
    node_t *current = list->head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

/* Append an element to the list */
void appendToList(list_t *list, elem value) {
    if (list == NULL) return;

    node_t *newNode = createNode(value);
    if (newNode == NULL) return; // Handle allocation failure

    if (list->head == NULL) {
        list->head = newNode;
    } else {
        node_t *current = list->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
}

/* Prepend an element to the list */
void prependToList(list_t *list, elem value) {
    if (list == NULL) return;

    node_t *newNode = createNode(value);
    if (newNode == NULL) return; // Handle allocation failure
    
    newNode->next = list->head;
    list->head = newNode;
}

/* Insert an element at a specific position */
void insertAtIndex(list_t *list, elem value, int index) {
    if (list == NULL) return;

    if (index <= 1) { // Treat index 0 and 1 as front
        prependToList(list, value);
        return;
    }

    node_t *newNode = createNode(value);
    if (newNode == NULL) return; // Handle allocation failure

    node_t *current = list->head;
    int i;
    for (i = 1; i < index - 1 && current != NULL; i++) {
        current = current->next;
    }

    if (current == NULL) {
        // Index beyond end, append to back
        free(newNode);
        appendToList(list, value);
    } else {
        newNode->next = current->next;
        current->next = newNode;
    }
}

/* Remove and return the last element */
elem removeFromBack(list_t *list) {
    if (list == NULL || list->head == NULL) return -1;

    node_t *current = list->head;
    node_t *previous = NULL;

    while (current->next != NULL) {
        previous = current;
        current = current->next;
    }

    elem value = current->value;
    if (previous == NULL) {
        // Only one element
        list->head = NULL;
    } else {
        previous->next = NULL;
    }

    free(current);
    return value;
}

/* Remove and return the first element */
elem removeFromFront(list_t *list) {
    if (list == NULL || list->head == NULL) return -1;

    node_t *toRemove = list->head;
    elem value = toRemove->value;
    
    list->head = toRemove->next;
    free(toRemove);

    return value;
}

/* Remove and return element at specific index */
elem removeAtIndex(list_t *list, int index) {
    if (list == NULL || list->head == NULL) return -1;

    if (index <= 0) return removeFromFront(list);

    node_t *current = list->head;
    node_t *previous = NULL;

    int i;
    for (i = 1; i < index && current != NULL; i++) {
        previous = current;
        current = current->next;
    }

    if (current == NULL) return -1;

    previous->next = current->next;
    elem value = current->value;
    free(current);
    return value;
}

/* Check if an element exists in the list */
bool isInList(list_t *list, elem value) { 
    if (list == NULL) return false;

    node_t *current = list->head;
    while (current != NULL) {
        if (current->value == value) return true;
        current = current->next;
    }
    return false;
}

/* Get element at a specific index */
elem getElementAt(list_t *list, int index) { 
    if (list == NULL || index <= 0) return -1; 

    node_t *current = list->head;
    int i;
    for (i = 1; i < index && current != NULL; i++) {
        current = current->next;
    }

    return (current == NULL) ? -1 : current->value;
}

/* Find index of first occurrence of an element */
int getIndexOf(list_t *list, elem value) { 
    if (list == NULL) return -1;

    node_t *current = list->head;
    int index = 1;
    while (current != NULL) {
        if (current->value == value) return index;
        current = current->next;
        index++;
    }
    return -1; 
}