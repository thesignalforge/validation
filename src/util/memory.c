/*
 * Memory management utilities
 */

#include "memory.h"

#define CLEANUP_INITIAL_CAPACITY 8

/* Initialize a cleanup stack */
void sf_cleanup_stack_init(sf_cleanup_stack_t *stack)
{
    stack->ptrs = NULL;
    stack->count = 0;
    stack->capacity = 0;
}

/* Push a pointer onto the cleanup stack */
void sf_cleanup_stack_push(sf_cleanup_stack_t *stack, void *ptr)
{
    if (ptr == NULL) {
        return;
    }

    if (stack->count >= stack->capacity) {
        size_t new_capacity = stack->capacity == 0 ? CLEANUP_INITIAL_CAPACITY : stack->capacity * 2;
        stack->ptrs = erealloc(stack->ptrs, new_capacity * sizeof(void *));
        stack->capacity = new_capacity;
    }

    stack->ptrs[stack->count++] = ptr;
}

/* Free all pointers in the cleanup stack and destroy it */
void sf_cleanup_stack_free_all(sf_cleanup_stack_t *stack)
{
    if (stack->ptrs) {
        for (size_t i = 0; i < stack->count; i++) {
            if (stack->ptrs[i]) {
                efree(stack->ptrs[i]);
            }
        }
        efree(stack->ptrs);
    }
    stack->ptrs = NULL;
    stack->count = 0;
    stack->capacity = 0;
}

/* Pop the last pointer from the stack (to prevent freeing it) */
void *sf_cleanup_stack_pop(sf_cleanup_stack_t *stack)
{
    if (stack->count == 0) {
        return NULL;
    }
    return stack->ptrs[--stack->count];
}

/* Destroy the stack without freeing the pointers */
void sf_cleanup_stack_destroy(sf_cleanup_stack_t *stack)
{
    if (stack->ptrs) {
        efree(stack->ptrs);
    }
    stack->ptrs = NULL;
    stack->count = 0;
    stack->capacity = 0;
}
