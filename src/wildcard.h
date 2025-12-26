/*
 * Wildcard expansion for nested field validation
 */

#ifndef SIGNALFORGE_WILDCARD_H
#define SIGNALFORGE_WILDCARD_H

#include "php_signalforge_validation.h"

/* Expanded field entry */
typedef struct {
    char *path;          /* Full path like "items.0.name" */
    size_t path_len;
    zval *value;         /* Pointer to the value in original data */
} sf_expanded_field_t;

/* Check if a field pattern contains wildcards */
zend_bool sf_has_wildcard(const char *pattern, size_t len);

/* Expand wildcards in a field pattern against data
 * Returns a HashTable of sf_expanded_field_t pointers
 * Pattern like "items.*.name" becomes "items.0.name", "items.1.name", etc.
 */
HashTable *sf_expand_wildcards(const char *pattern, size_t pattern_len, HashTable *data);

/* Get a nested value from data using dot notation
 * e.g., "user.address.city" from {'user': {'address': {'city': 'Zagreb'}}}
 */
zval *sf_get_nested_value(const char *path, size_t path_len, HashTable *data);

/* Free expanded fields hashtable */
void sf_free_expanded_fields(HashTable *expanded);

#endif /* SIGNALFORGE_WILDCARD_H */
