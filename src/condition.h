/*
 * Condition evaluation
 */

#ifndef SIGNALFORGE_CONDITION_H
#define SIGNALFORGE_CONDITION_H

#include "php_signalforge_validation.h"

/* Condition operators */
typedef enum {
    COND_OP_EQ,        /* = */
    COND_OP_NEQ,       /* != */
    COND_OP_GT,        /* > */
    COND_OP_GTE,       /* >= */
    COND_OP_LT,        /* < */
    COND_OP_LTE,       /* <= */
    COND_OP_IN,        /* in */
    COND_OP_NOT_IN,    /* not_in */
    COND_OP_FILLED,    /* filled (unary) */
    COND_OP_EMPTY,     /* empty (unary) */
    COND_OP_MATCHES,   /* matches regex */
} sf_condition_op_t;

/* Condition subjects */
typedef enum {
    SUBJECT_SELF_LENGTH,   /* @length */
    SUBJECT_SELF_VALUE,    /* @value */
    SUBJECT_SELF_TYPE,     /* @type */
    SUBJECT_SELF_EMPTY,    /* @empty */
    SUBJECT_SELF_FILLED,   /* @filled */
    SUBJECT_SELF_MATCHES,  /* @matches */
    SUBJECT_OTHER_FIELD,   /* field name */
} sf_condition_subject_t;

/* Condition kind */
typedef enum {
    COND_SIMPLE,
    COND_AND,
    COND_OR,
} sf_condition_kind_t;

/* Forward declaration */
struct sf_condition_s;

/* Condition structure */
typedef struct sf_condition_s {
    sf_condition_kind_t kind;
    union {
        /* Simple condition */
        struct {
            sf_condition_subject_t subject;
            char *field_name;      /* For SUBJECT_OTHER_FIELD */
            size_t field_len;
            sf_condition_op_t op;
            zval value;            /* The value to compare against */
        } simple;

        /* Compound condition (AND/OR) */
        struct {
            struct sf_condition_s **conditions;
            size_t count;
        } compound;
    };
} sf_condition_t;

/* Parse a condition from PHP array */
sf_condition_t *sf_parse_condition(zval *condition_array);

/* Evaluate a condition */
zend_bool sf_evaluate_condition(
    sf_condition_t *cond,
    zval *current_value,
    HashTable *all_data,
    const char *current_field,
    signalforge_validator_t *validator
);

/* Free a condition */
void sf_free_condition(sf_condition_t *cond);

/* Helper to check if a value is considered "filled" */
zend_bool sf_is_filled(zval *value);

/* Helper to check if a value is considered "empty" */
zend_bool sf_is_empty(zval *value);

#endif /* SIGNALFORGE_CONDITION_H */
