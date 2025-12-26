/*
 * Wildcard expansion for nested field validation
 */

#include "wildcard.h"
#include <string.h>

/* Check if a field pattern contains wildcards */
zend_bool sf_has_wildcard(const char *pattern, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        if (pattern[i] == '*') {
            return 1;
        }
    }
    return 0;
}

/* Get segment from path (up to next dot or end) */
static size_t get_segment(const char *path, size_t len, const char **segment, size_t *seg_len)
{
    const char *dot = memchr(path, '.', len);
    if (dot) {
        *segment = path;
        *seg_len = dot - path;
        return *seg_len + 1; /* Include the dot */
    }
    *segment = path;
    *seg_len = len;
    return len;
}

/* Get nested value from HashTable */
zval *sf_get_nested_value(const char *path, size_t path_len, HashTable *data)
{
    if (!path || path_len == 0 || !data) {
        return NULL;
    }

    const char *remaining = path;
    size_t remaining_len = path_len;
    HashTable *current_ht = data;
    zval *current_val = NULL;

    while (remaining_len > 0) {
        const char *segment;
        size_t seg_len;
        size_t consumed = get_segment(remaining, remaining_len, &segment, &seg_len);

        /* Try string key first */
        zend_string *key = zend_string_init(segment, seg_len, 0);
        current_val = zend_hash_find(current_ht, key);
        zend_string_release(key);

        /* Try numeric key if string key failed */
        if (!current_val) {
            char *endptr;
            long index = strtol(segment, &endptr, 10);
            if (endptr == segment + seg_len) {
                current_val = zend_hash_index_find(current_ht, index);
            }
        }

        if (!current_val) {
            return NULL;
        }

        remaining += consumed;
        remaining_len -= consumed;

        if (remaining_len > 0) {
            if (Z_TYPE_P(current_val) != IS_ARRAY) {
                return NULL;
            }
            current_ht = Z_ARRVAL_P(current_val);
        }
    }

    return current_val;
}

/* Recursive wildcard expansion helper */
static void expand_wildcards_recursive(
    const char *pattern,
    size_t pattern_len,
    HashTable *data,
    const char *prefix,
    size_t prefix_len,
    HashTable *result
)
{
    if (pattern_len == 0) {
        /* End of pattern - add the current path */
        sf_expanded_field_t *entry = emalloc(sizeof(sf_expanded_field_t));
        entry->path = estrndup(prefix, prefix_len);
        entry->path_len = prefix_len;
        entry->value = sf_get_nested_value(prefix, prefix_len, result);

        zend_string *key = zend_string_init(prefix, prefix_len, 0);
        zend_hash_add_ptr(result, key, entry);
        zend_string_release(key);
        return;
    }

    const char *segment;
    size_t seg_len;
    size_t consumed = get_segment(pattern, pattern_len, &segment, &seg_len);

    /* Get current data at prefix */
    zval *current_data = NULL;
    if (prefix_len > 0) {
        current_data = sf_get_nested_value(prefix, prefix_len, data);
    } else {
        /* Create a temporary zval for the root data */
        zval temp;
        ZVAL_ARR(&temp, data);
        current_data = &temp;
    }

    if (!current_data && prefix_len > 0) {
        return;
    }

    HashTable *current_ht = (prefix_len > 0 && current_data) ?
        (Z_TYPE_P(current_data) == IS_ARRAY ? Z_ARRVAL_P(current_data) : NULL) : data;

    if (!current_ht) {
        return;
    }

    if (seg_len == 1 && segment[0] == '*') {
        /* Wildcard - iterate over all keys */
        zend_string *key;
        zend_ulong idx;

        ZEND_HASH_FOREACH_KEY(current_ht, idx, key) {
            char new_prefix[4096];
            size_t new_prefix_len;

            if (key) {
                if (prefix_len > 0) {
                    new_prefix_len = snprintf(new_prefix, sizeof(new_prefix), "%.*s.%s",
                        (int)prefix_len, prefix, ZSTR_VAL(key));
                } else {
                    new_prefix_len = snprintf(new_prefix, sizeof(new_prefix), "%s", ZSTR_VAL(key));
                }
            } else {
                if (prefix_len > 0) {
                    new_prefix_len = snprintf(new_prefix, sizeof(new_prefix), "%.*s.%lu",
                        (int)prefix_len, prefix, idx);
                } else {
                    new_prefix_len = snprintf(new_prefix, sizeof(new_prefix), "%lu", idx);
                }
            }

            const char *remaining = pattern + consumed;
            size_t remaining_len = pattern_len - consumed;

            if (remaining_len > 0) {
                expand_wildcards_recursive(remaining, remaining_len, data, new_prefix, new_prefix_len, result);
            } else {
                /* End of pattern */
                sf_expanded_field_t *entry = emalloc(sizeof(sf_expanded_field_t));
                entry->path = estrndup(new_prefix, new_prefix_len);
                entry->path_len = new_prefix_len;
                entry->value = sf_get_nested_value(new_prefix, new_prefix_len, data);

                zend_string *result_key = zend_string_init(new_prefix, new_prefix_len, 0);
                zend_hash_add_ptr(result, result_key, entry);
                zend_string_release(result_key);
            }
        } ZEND_HASH_FOREACH_END();
    } else {
        /* Regular segment - just append and continue */
        char new_prefix[4096];
        size_t new_prefix_len;

        if (prefix_len > 0) {
            new_prefix_len = snprintf(new_prefix, sizeof(new_prefix), "%.*s.%.*s",
                (int)prefix_len, prefix, (int)seg_len, segment);
        } else {
            new_prefix_len = snprintf(new_prefix, sizeof(new_prefix), "%.*s", (int)seg_len, segment);
        }

        const char *remaining = pattern + consumed;
        size_t remaining_len = pattern_len - consumed;

        expand_wildcards_recursive(remaining, remaining_len, data, new_prefix, new_prefix_len, result);
    }
}

/* Expand wildcards in a field pattern against data */
HashTable *sf_expand_wildcards(const char *pattern, size_t pattern_len, HashTable *data)
{
    HashTable *result;
    ALLOC_HASHTABLE(result);
    zend_hash_init(result, 8, NULL, NULL, 0);

    if (!sf_has_wildcard(pattern, pattern_len)) {
        /* No wildcards - just add the pattern as-is */
        sf_expanded_field_t *entry = emalloc(sizeof(sf_expanded_field_t));
        entry->path = estrndup(pattern, pattern_len);
        entry->path_len = pattern_len;
        entry->value = sf_get_nested_value(pattern, pattern_len, data);

        zend_string *key = zend_string_init(pattern, pattern_len, 0);
        zend_hash_add_ptr(result, key, entry);
        zend_string_release(key);
    } else {
        expand_wildcards_recursive(pattern, pattern_len, data, "", 0, result);
    }

    return result;
}

/* Free expanded field entry */
static void free_expanded_field(sf_expanded_field_t *entry)
{
    if (entry) {
        if (entry->path) {
            efree(entry->path);
        }
        efree(entry);
    }
}

/* Free expanded fields hashtable */
void sf_free_expanded_fields(HashTable *expanded)
{
    if (!expanded) return;

    sf_expanded_field_t *entry;
    ZEND_HASH_FOREACH_PTR(expanded, entry) {
        free_expanded_field(entry);
    } ZEND_HASH_FOREACH_END();

    zend_hash_destroy(expanded);
    FREE_HASHTABLE(expanded);
}
