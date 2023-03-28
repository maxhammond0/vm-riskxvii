#include <stdint.h>

#ifndef HBANK_H_
#define HBANK_H_

typedef struct node node_t;

/*
heap_init
Creates a heap by creating a head node
:: uint64_t value :: The value to be stored in the head node
Returns a pointer to the newly created node
*/
struct node* heap_init(uint64_t data);

/*
heap_add
Adds a node containing a specified value to the existing heap
:: uint64_t value :: The value to be stored in the new node
Does not return anything
*/
void heap_add(node_t** head, uint64_t data);

/*
heap_delete
Removes the specified node from the list and updates the list
accordingly
:: node_t* n :: The pointer to the node to be deleted
Does not return anything
*/
void heap_delete(node_t** head, node_t* n);

/*
heap_free
Frees all existing nodes in the virtual heap
:: node_t** head :: The pointer to the pointer that is
pointing to the head node of the heap
Does not return anything
*/
void heap_free(node_t** head);

#endif
