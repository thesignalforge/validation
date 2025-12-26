/*
 * Condition evaluation
 */

#include "condition.h"
#include "util/utf8.h"

/* Check if a value is considered "empty" */
zend_bool sf_is_empty(zval *value)
{
    if (value == NULL) {
        return 1;
    }

    switch (Z_TYPE_P(value)) {
        case IS_NULL:
            return 1;

        case IS_FALSE:
            return 1;

        case IS_TRUE:
            return 0;

        case IS_LONG:
            return 0;

        case IS_DOUBLE:
            return 0;

        case IS_STRING:
            return Z_STRLEN_P(value) == 0;

        case IS_ARRAY:
            return zend_hash_num_elements(Z_ARRVAL_P(value)) == 0;

        default:
            return 0;
    }
}

/* Check if a value is considered "filled" */
zend_bool sf_is_filled(zval *value)
{
    return !sf_is_empty(value);
}

/* Get operator from string */
static sf_condition_op_t parse_operator(const char *op, size_t len)
{
    if (len == 1) {
        if (op[0] == '=') return COND_OP_EQ;
        if (op[0] == '>') return COND_OP_GT;
        if (op[0] == '<') return COND_OP_LT;
    } else if (len == 2) {
        if (memcmp(op, "!=", 2) == 0) return COND_OP_NEQ;
        if (memcmp(op, ">=", 2) == 0) return COND_OP_GTE;
        if (memcmp(op, "<=", 2) == 0) return COND_OP_LTE;
        if (memcmp(op, "in", 2) == 0) return COND_OP_IN;
    } else if (len == 5) {
        if (memcmp(op, "empty", 5) == 0) return COND_OP_EMPTY;
    } else if (len == 6) {
        if (memcmp(op, "not_in", 6) == 0) return COND_OP_NOT_IN;
        if (memcmp(op, "filled", 6) == 0) return COND_OP_FILLED;
    } else if (len == 7) {
        if (memcmp(op, "matches", 7) == 0) return COND_OP_MATCHES;
    }

    return COND_OP_EQ; /* Default */
}

/* Parse a condition from PHP array */
sf_condition_t *sf_parse_condition(zval *condition_array)
{
    if (Z_TYPE_P(condition_array) != IS_ARRAY) {
        return NULL;
    }

    HashTable *arr = Z_ARRVAL_P(condition_array);
    zval *first = zend_hash_index_find(arr, 0);

    if (!first || Z_TYPE_P(first) != IS_STRING) {
        return NULL;
    }

    zend_string *first_str = Z_STR_P(first);
    const char *first_val = ZSTR_VAL(first_str);
    size_t first_len = ZSTR_LEN(first_str);

    sf_condition_t *cond = ecalloc(1, sizeof(sf_condition_t));

    /* Check for compound conditions: and, or */
    if (first_len == 3 && memcmp(first_val, "and", 3) == 0) {
        cond->kind = COND_AND;
        size_t count = zend_hash_num_elements(arr) - 1;
        cond->compound.conditions = ecalloc(count, sizeof(sf_condition_t *));
        cond->compound.count = 0;

        for (size_t i = 1; i <= count; i++) {
            zval *sub = zend_hash_index_find(arr, i);
            if (sub && Z_TYPE_P(sub) == IS_ARRAY) {
                sf_condition_t *sub_cond = sf_parse_condition(sub);
                if (sub_cond) {
                    cond->compound.conditions[cond->compound.count++] = sub_cond;
                }
            }
        }
        return cond;
    }

    if (first_len == 2 && memcmp(first_val, "or", 2) == 0) {
        cond->kind = COND_OR;
        size_t count = zend_hash_num_elements(arr) - 1;
        cond->compound.conditions = ecalloc(count, sizeof(sf_condition_t *));
        cond->compound.count = 0;

        for (size_t i = 1; i <= count; i++) {
            zval *sub = zend_hash_index_find(arr, i);
            if (sub && Z_TYPE_P(sub) == IS_ARRAY) {
                sf_condition_t *sub_cond = sf_parse_condition(sub);
                if (sub_cond) {
                    cond->compound.conditions[cond->compound.count++] = sub_cond;
                }
            }
        }
        return cond;
    }

    /* Simple condition */
    cond->kind = COND_SIMPLE;

    /* Check for self-referential subjects starting with @ */
    if (first_val[0] == '@') {
        if (first_len == 7 && memcmp(first_val, "@length", 7) == 0) {
            cond->simple.subject = SUBJECT_SELF_LENGTH;
        } else if (first_len == 6 && memcmp(first_val, "@value", 6) == 0) {
            cond->simple.subject = SUBJECT_SELF_VALUE;
        } else if (first_len == 5 && memcmp(first_val, "@type", 5) == 0) {
            cond->simple.subject = SUBJECT_SELF_TYPE;
        } else if (first_len == 6 && memcmp(first_val, "@empty", 6) == 0) {
            cond->simple.subject = SUBJECT_SELF_EMPTY;
            cond->simple.op = COND_OP_EMPTY;
            efree(cond);
            cond = ecalloc(1, sizeof(sf_condition_t));
            cond->kind = COND_SIMPLE;
            cond->simple.subject = SUBJECT_SELF_EMPTY;
            return cond;
        } else if (first_len == 7 && memcmp(first_val, "@filled", 7) == 0) {
            cond->simple.subject = SUBJECT_SELF_FILLED;
            efree(cond);
            cond = ecalloc(1, sizeof(sf_condition_t));
            cond->kind = COND_SIMPLE;
            cond->simple.subject = SUBJECT_SELF_FILLED;
            return cond;
        } else if (first_len == 8 && memcmp(first_val, "@matches", 8) == 0) {
            cond->simple.subject = SUBJECT_SELF_MATCHES;
            zval *pattern = zend_hash_index_find(arr, 1);
            if (pattern && Z_TYPE_P(pattern) == IS_STRING) {
                ZVAL_COPY(&cond->simple.value, pattern);
            }
            return cond;
        } else {
            efree(cond);
            return NULL;
        }
    } else {
        /* Cross-field reference */
        cond->simple.subject = SUBJECT_OTHER_FIELD;
        cond->simple.field_name = estrndup(first_val, first_len);
        cond->simple.field_len = first_len;
    }

    /* Get operator */
    zval *op_zval = zend_hash_index_find(arr, 1);
    if (op_zval && Z_TYPE_P(op_zval) == IS_STRING) {
        cond->simple.op = parse_operator(Z_STRVAL_P(op_zval), Z_STRLEN_P(op_zval));

        /* Check for unary operators */
        if (cond->simple.op == COND_OP_FILLED || cond->simple.op == COND_OP_EMPTY) {
            return cond;
        }
    } else {
        cond->simple.op = COND_OP_EQ;
    }

    /* Get value */
    zval *value_zval = zend_hash_index_find(arr, 2);
    if (value_zval) {
        ZVAL_COPY(&cond->simple.value, value_zval);
    } else {
        ZVAL_NULL(&cond->simple.value);
    }

    return cond;
}

/* Compare two zval values */
static int compare_zvals(zval *a, zval *b)
{
    zval result;
    if (compare_function(&result, a, b) == SUCCESS) {
        return (int)Z_LVAL(result);
    }
    return 0;
}

/* Check if value is in array */
static zend_bool value_in_array(zval *value, zval *array_val)
{
    if (Z_TYPE_P(array_val) != IS_ARRAY) {
        return 0;
    }

    HashTable *arr = Z_ARRVAL_P(array_val);
    zval *item;

    ZEND_HASH_FOREACH_VAL(arr, item) {
        if (compare_zvals(value, item) == 0) {
            return 1;
        }
    } ZEND_HASH_FOREACH_END();

    return 0;
}

/* Evaluate a simple condition */
static zend_bool evaluate_simple_condition(
    sf_condition_t *cond,
    zval *current_value,
    HashTable *all_data,
    const char *current_field,
    signalforge_validator_t *validator
)
{
    zval subject_value;
    zval *subject_ptr = NULL;

    switch (cond->simple.subject) {
        case SUBJECT_SELF_LENGTH: {
            if (current_value && Z_TYPE_P(current_value) == IS_STRING) {
                size_t len = sf_utf8_strlen(Z_STRVAL_P(current_value), Z_STRLEN_P(current_value));
                ZVAL_LONG(&subject_value, (zend_long)len);
            } else if (current_value && Z_TYPE_P(current_value) == IS_ARRAY) {
                ZVAL_LONG(&subject_value, (zend_long)zend_hash_num_elements(Z_ARRVAL_P(current_value)));
            } else {
                ZVAL_LONG(&subject_value, 0);
            }
            subject_ptr = &subject_value;
            break;
        }

        case SUBJECT_SELF_VALUE:
            subject_ptr = current_value;
            break;

        case SUBJECT_SELF_TYPE: {
            if (!current_value) {
                ZVAL_STRING(&subject_value, "null");
            } else {
                switch (Z_TYPE_P(current_value)) {
                    case IS_NULL: ZVAL_STRING(&subject_value, "null"); break;
                    case IS_TRUE:
                    case IS_FALSE: ZVAL_STRING(&subject_value, "boolean"); break;
                    case IS_LONG: ZVAL_STRING(&subject_value, "integer"); break;
                    case IS_DOUBLE: ZVAL_STRING(&subject_value, "double"); break;
                    case IS_STRING: ZVAL_STRING(&subject_value, "string"); break;
                    case IS_ARRAY: ZVAL_STRING(&subject_value, "array"); break;
                    case IS_OBJECT: ZVAL_STRING(&subject_value, "object"); break;
                    default: ZVAL_STRING(&subject_value, "unknown"); break;
                }
            }
            subject_ptr = &subject_value;
            break;
        }

        case SUBJECT_SELF_EMPTY:
            return sf_is_empty(current_value);

        case SUBJECT_SELF_FILLED:
            return sf_is_filled(current_value);

        case SUBJECT_SELF_MATCHES: {
            if (!current_value || Z_TYPE_P(current_value) != IS_STRING) {
                return 0;
            }
            if (Z_TYPE(cond->simple.value) != IS_STRING) {
                return 0;
            }

            /* Use PHP's preg_match */
            pcre2_code *re;
            PCRE2_SIZE erroffset;
            int errcode;

            re = pcre2_compile(
                (PCRE2_SPTR)Z_STRVAL(cond->simple.value),
                Z_STRLEN(cond->simple.value),
                PCRE2_UTF,
                &errcode,
                &erroffset,
                NULL
            );

            if (!re) {
                return 0;
            }

            pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(re, NULL);
            int rc = pcre2_match(
                re,
                (PCRE2_SPTR)Z_STRVAL_P(current_value),
                Z_STRLEN_P(current_value),
                0,
                0,
                match_data,
                NULL
            );

            pcre2_match_data_free(match_data);
            pcre2_code_free(re);

            return rc >= 0;
        }

        case SUBJECT_OTHER_FIELD: {
            zend_string *field_key = zend_string_init(cond->simple.field_name, cond->simple.field_len, 0);
            subject_ptr = zend_hash_find(all_data, field_key);
            zend_string_release(field_key);
            break;
        }
    }

    /* Apply operator */
    switch (cond->simple.op) {
        case COND_OP_EQ:
            return compare_zvals(subject_ptr, &cond->simple.value) == 0;

        case COND_OP_NEQ:
            return compare_zvals(subject_ptr, &cond->simple.value) != 0;

        case COND_OP_GT:
            return compare_zvals(subject_ptr, &cond->simple.value) > 0;

        case COND_OP_GTE:
            return compare_zvals(subject_ptr, &cond->simple.value) >= 0;

        case COND_OP_LT:
            return compare_zvals(subject_ptr, &cond->simple.value) < 0;

        case COND_OP_LTE:
            return compare_zvals(subject_ptr, &cond->simple.value) <= 0;

        case COND_OP_IN:
            return value_in_array(subject_ptr, &cond->simple.value);

        case COND_OP_NOT_IN:
            return !value_in_array(subject_ptr, &cond->simple.value);

        case COND_OP_FILLED:
            return sf_is_filled(subject_ptr);

        case COND_OP_EMPTY:
            return sf_is_empty(subject_ptr);

        case COND_OP_MATCHES:
            /* Already handled above for self-referential */
            break;
    }

    /* Cleanup temporary values */
    if (cond->simple.subject == SUBJECT_SELF_TYPE) {
        zval_ptr_dtor(&subject_value);
    }

    return 0;
}

/* Evaluate a condition */
zend_bool sf_evaluate_condition(
    sf_condition_t *cond,
    zval *current_value,
    HashTable *all_data,
    const char *current_field,
    signalforge_validator_t *validator
)
{
    if (!cond) {
        return 1;
    }

    switch (cond->kind) {
        case COND_SIMPLE:
            return evaluate_simple_condition(cond, current_value, all_data, current_field, validator);

        case COND_AND: {
            for (size_t i = 0; i < cond->compound.count; i++) {
                if (!sf_evaluate_condition(cond->compound.conditions[i], current_value, all_data, current_field, validator)) {
                    return 0;
                }
            }
            return 1;
        }

        case COND_OR: {
            for (size_t i = 0; i < cond->compound.count; i++) {
                if (sf_evaluate_condition(cond->compound.conditions[i], current_value, all_data, current_field, validator)) {
                    return 1;
                }
            }
            return 0;
        }
    }

    return 0;
}

/* Free a condition */
void sf_free_condition(sf_condition_t *cond)
{
    if (!cond) return;

    if (cond->kind == COND_SIMPLE) {
        if (cond->simple.field_name) {
            efree(cond->simple.field_name);
        }
        zval_ptr_dtor(&cond->simple.value);
    } else {
        for (size_t i = 0; i < cond->compound.count; i++) {
            sf_free_condition(cond->compound.conditions[i]);
        }
        if (cond->compound.conditions) {
            efree(cond->compound.conditions);
        }
    }

    efree(cond);
}
