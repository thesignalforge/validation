/*
 * Rule array parser
 */

#ifndef SIGNALFORGE_PARSER_H
#define SIGNALFORGE_PARSER_H

#include "php_signalforge_validation.h"
#include "condition.h"

/* Rule types */
typedef enum {
    /* Presence rules */
    RULE_REQUIRED,
    RULE_NULLABLE,
    RULE_FILLED,
    RULE_PRESENT,

    /* Type rules */
    RULE_STRING,
    RULE_INTEGER,
    RULE_NUMERIC,
    RULE_BOOLEAN,
    RULE_ARRAY,

    /* String rules */
    RULE_MIN,
    RULE_MAX,
    RULE_BETWEEN,
    RULE_REGEX,
    RULE_NOT_REGEX,
    RULE_ALPHA,
    RULE_ALPHA_NUM,
    RULE_ALPHA_DASH,
    RULE_LOWERCASE,
    RULE_UPPERCASE,
    RULE_STARTS_WITH,
    RULE_ENDS_WITH,
    RULE_CONTAINS,

    /* Numeric rules (reuse MIN, MAX, BETWEEN) */
    RULE_GT,
    RULE_GTE,
    RULE_LT,
    RULE_LTE,

    /* Array rules (reuse MIN, MAX, BETWEEN) */
    RULE_DISTINCT,

    /* Format rules */
    RULE_EMAIL,
    RULE_URL,
    RULE_IP,
    RULE_UUID,
    RULE_JSON,
    RULE_DATE,
    RULE_DATE_FORMAT,
    RULE_AFTER,
    RULE_BEFORE,
    RULE_AFTER_OR_EQUAL,
    RULE_BEFORE_OR_EQUAL,

    /* Comparison rules */
    RULE_IN,
    RULE_NOT_IN,
    RULE_SAME,
    RULE_DIFFERENT,
    RULE_CONFIRMED,

    /* Regional rules */
    RULE_OIB,
    RULE_PHONE,
    RULE_IBAN,
    RULE_VAT_EU,

    /* Conditional */
    RULE_WHEN,

    /* Sentinel */
    RULE_UNKNOWN
} sf_rule_type_t;

/* Forward declaration */
struct sf_parsed_rule_s;

/* Parsed rule structure */
typedef struct sf_parsed_rule_s {
    sf_rule_type_t type;
    union {
        /* For min, max, gt, gte, lt, lte */
        struct {
            zend_long value;
        } size;

        /* For between */
        struct {
            zend_long min;
            zend_long max;
        } range;

        /* For regex, not_regex */
        struct {
            char *pattern;
            size_t len;
        } regex;

        /* For starts_with, ends_with, contains, date_format */
        struct {
            char *str;
            size_t len;
        } string;

        /* For same, different, confirmed, after, before */
        struct {
            char *field;
            size_t len;
        } field_ref;

        /* For in, not_in */
        struct {
            HashTable *values;
        } in_list;

        /* For when conditional */
        struct {
            sf_condition_t *condition;
            struct sf_parsed_rule_s **then_rules;
            size_t then_count;
            struct sf_parsed_rule_s **else_rules;
            size_t else_count;
        } conditional;
    } params;
} sf_parsed_rule_t;

/* Field rules structure */
typedef struct {
    char *field_name;
    size_t field_len;
    sf_parsed_rule_t **rules;
    size_t rule_count;
} sf_field_rules_t;

/* Parse rules from PHP array */
HashTable *sf_parse_rules(HashTable *rules_array);

/* Free parsed rules */
void sf_free_field_rules(sf_field_rules_t *field_rules);
void sf_free_parsed_rule(sf_parsed_rule_t *rule);
void sf_free_parsed_rules_ht(HashTable *rules);

/* Validate rule name */
zend_bool sf_validate_rule_name(const char *name, size_t len);

/* Get rule type from string */
sf_rule_type_t sf_get_rule_type(const char *name, size_t len);

#endif /* SIGNALFORGE_PARSER_H */
