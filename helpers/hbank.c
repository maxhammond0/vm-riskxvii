#include <stdlib.h>
#include <stdint.h>

#include "hbank.h"

struct node {
    uint64_t data;
    node_t* next;
};


node_t* heap_init(uint64_t data) {
    node_t *head = malloc(sizeof(node_t));
    head->data = 0;
    head->next = NULL;

    return head;
}

void heap_add(node_t** head, uint64_t data) {
    node_t* new_node = heap_init(data);

    node_t* prev = NULL;
    node_t* cursor = *head;

    if (cursor == NULL) {
        *head = new_node;
    } else {
        while (cursor != NULL) {
            prev = cursor;
            cursor = cursor->next;
        }

        prev->next = new_node;
    }
}

void heap_delete(node_t** head, node_t* n) {
    node_t* cursor = *head;
    node_t* prev = NULL;
    node_t* tmp = NULL;

    if (*head == n) {
        tmp = *head;
        *head = (*head)->next;
        free(tmp);
    } else {
        while (cursor != NULL && cursor != n) {
            prev = cursor;
            cursor = cursor->next;
        }

        if (cursor) {
            tmp = cursor;
            prev->next = cursor->next;
            free(tmp);
        }
    }
}

void heap_free(node_t** head) {
    node_t* tmp = NULL;

    while (*head != NULL) {
        tmp = *head;
        *head = (*head)->next;
        free(tmp);
    }
}
