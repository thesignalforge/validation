/*
 * Memory management utilities
 */

#ifndef SIGNALFORGE_MEMORY_H
#define SIGNALFORGE_MEMORY_H

#include "php.h"

/* Cleanup stack for tracking allocations in complex operations */
typedef struct {
    void **ptrs;
    size_t count;
    size_t capacity;
} sf_cleanup_stack_t;

/* Initialize a cleanup stack */
void sf_cleanup_stack_init(sf_cleanup_stack_t *stack);

/* Push a pointer onto the cleanup stack */
void sf_cleanup_stack_push(sf_cleanup_stack_t *stack, void *ptr);

/* Free all pointers in the cleanup stack and destroy it */
void sf_cleanup_stack_free_all(sf_cleanup_stack_t *stack);

/* Pop the last pointer from the stack (to prevent freeing it) */
void *sf_cleanup_stack_pop(sf_cleanup_stack_t *stack);

/* Destroy the stack without freeing the pointers */
void sf_cleanup_stack_destroy(sf_cleanup_stack_t *stack);

/* Helper macros */
#define SF_CLEANUP_INIT(stack) sf_cleanup_stack_init(&(stack))
#define SF_CLEANUP_PUSH(stack, ptr) sf_cleanup_stack_push(&(stack), (ptr))
#define SF_CLEANUP_ALL(stack) sf_cleanup_stack_free_all(&(stack))
#define SF_CLEANUP_POP(stack) sf_cleanup_stack_pop(&(stack))
#define SF_CLEANUP_DESTROY(stack) sf_cleanup_stack_destroy(&(stack))

#endif /* SIGNALFORGE_MEMORY_H */
